#include "ecaio_proc.h"

#if defined __MS_IOCP

//*****************************************************************************

#include <windows.h>
#include <stdio.h>

//-----------------------------------------------------------------------------

struct EcAioProc_s
{
  
  
  
};

//-----------------------------------------------------------------------------

EcAioProc ecaio_proc_create (int pid)
{
  
}

//-----------------------------------------------------------------------------

int ecaio_proc_assign (EcAioProc* pself, EcAio aio, EcErr err)
{
  
}

//*****************************************************************************

#elif defined __BSD_KEVENT2

//*****************************************************************************

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>

#include <sys/event.h>

//-----------------------------------------------------------------------------

struct EcAioProc_s
{
  long pid;
  
  fct_ecaio_context_destroy onDestroy;
  
  fct_ecaio_context_onNotify onNotify;
  
  void* ptr;
  
};

//-----------------------------------------------------------------------------

EcAioProc ecaio_proc_create (int pid)
{
  EcAioProc self = ENTC_NEW(struct EcAioProc_s);
  
  self->pid = pid;
  
  self->onNotify = NULL;
  self->onDestroy = NULL;
  self->ptr = NULL;
  
  return self;
}

//-----------------------------------------------------------------------------

void ecaio_proc_destroy (EcAioProc* pself)
{
  ENTC_DEL(pself, struct EcAioProc_s);
}

//-----------------------------------------------------------------------------

static int __STDCALL ecaio_proc_onProcess (void* ptr, EcAioContext ctx, unsigned long flags, unsigned long filter)
{
  EcAioProc self = ptr;
  
  if (self->onNotify)
  {
    self->onNotify (self->ptr, flags);
  }
  
  return ENTC_AIO_CODE_ONCE;
}

//-----------------------------------------------------------------------------

static void __STDCALL ecaio_proc_onDestroy (void* ptr)
{
  EcAioProc self = ptr;
  
  if (self->onDestroy)
  {
    self->onDestroy (self->ptr);
  }
  
  ecaio_proc_destroy (&self);
}

//-----------------------------------------------------------------------------

int ecaio_proc_assign (EcAioProc* pself, EcAio aio, EcErr err)
{
  int res = ENTC_ERR_NONE;
  EcAioProc self = *pself;
  
  // create a async context
  EcAioContext ctx = ecaio_context_create ();
  
  // override callbacks
  ecaio_context_setCallbacks (ctx, self, ecaio_proc_onProcess, ecaio_proc_onDestroy);
  
  res = ecaio_appendPNode (aio, self->pid, ctx, err);
  if (res)
  {
    ecaio_proc_destroy (pself);
    return res;
  }
  
  *pself = NULL;
  return res;
}

//*****************************************************************************

#else

//*****************************************************************************

#include "ecthread.h"

//-----------------------------------------------------------------------------

struct EcAioProc_s
{
  
  void* ptr;
  
  fct_ecaio_context_destroy onDestroy;
  
  fct_ecaio_context_onNotify onNotify;
  
  int pid;
  
  EcThread thread;
  
};

//-----------------------------------------------------------------------------

static int __STDCALL ecaio_proc_thread (void* ptr)
{
  EcAioProc self = ptr;
  int res;
  
  waitpid(self->pid, &res, 0);
  
  if (self->onNotify)
  {
    self->onNotify (self->ptr, res);
  }
  
  if (self->onDestroy)
  {
    self->onDestroy (self->ptr);
  }
  
  ecthread_delete (&(self->thread));
  
  ENTC_DEL(&self, struct EcAioProc_s);
  
  return 0;
}

//-----------------------------------------------------------------------------

EcAioProc ecaio_proc_create (int pid)
{
  EcAioProc self = ENTC_NEW(struct EcAioProc_s);
  
  self->pid = pid;
  
  self->onNotify = NULL;
  self->onDestroy = NULL;
  self->ptr = NULL;
  
  self->thread = NULL;
  
  return self;
}

//-----------------------------------------------------------------------------

int ecaio_proc_assign (EcAioProc* pself, EcAio aio, EcErr err)
{
  EcAioProc self = *pself;
  
  self->thread = ecthread_new ();
  
  ecthread_start(self->thread, ecaio_proc_thread, self);
  
  *pself = NULL;
  
  return ENTC_ERR_NONE;
}

#endif

//*****************************************************************************

void ecaio_proc_setCallback (EcAioProc self, void* ptr, fct_ecaio_context_onNotify onNotify, fct_ecaio_context_destroy onDestroy)
{
  self->ptr = ptr;
  self->onDestroy = onDestroy;
  self->onNotify = onNotify;
}

//-----------------------------------------------------------------------------

