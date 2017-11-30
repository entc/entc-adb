#include "ecaio_proc.h"

#if defined __MS_IOCP

//*****************************************************************************

#include <windows.h>
#include <stdio.h>
#include "ecthread.h"
#include "ecaio_event.h"

//-----------------------------------------------------------------------------

struct EcAioProc_s
{
  
  void* ptr;
  
  fct_ecaio_context_destroy onDestroy;
  
  fct_ecaio_context_onNotify onNotify;
  
  HANDLE handle;
  
  EcThread thread;
  
  EcAio aio;
  
  void* eventh;
  
};

//-----------------------------------------------------------------------------

static int __STDCALL ecaio_proc_thread (void* ptr)
{
  EcAioProc self = ptr;
  
  // wait here until something happens
  WaitForSingleObject (self->handle, INFINITE);
  
  {
    EcErr err = ecerr_create ();
    
    ecaio_triggerENode (self->aio, self->eventh, err);
    
    ecerr_destroy (&err);
  }
  
  return 0;
}

//-----------------------------------------------------------------------------

static void __STDCALL ecaio_proc_onDestroy (void* ptr)
{
  EcAioProc self = ptr;
  
  if (self->onNotify)
  {
    self->onNotify (self->ptr, 0);
  }
  
  if (self->onDestroy)
  {
    self->onDestroy (self->ptr);
  }
  
  ecthread_join (self->thread);
  
  ecthread_delete (&(self->thread));
  
  ENTC_DEL(&self, struct EcAioProc_s);
}

//-----------------------------------------------------------------------------

EcAioProc ecaio_proc_create (uint64_t pid)
{
  EcAioProc self = ENTC_NEW(struct EcAioProc_s);
  
  self->handle = (void*)pid;
  
  self->onNotify = NULL;
  self->onDestroy = NULL;
  self->ptr = NULL;
  
  self->thread = NULL;
  
  return self;
}

//-----------------------------------------------------------------------------

int ecaio_proc_assign (EcAioProc* pself, EcAio aio, EcErr err)
{
  int res;
  EcAioProc self = *pself;
  
  // register a term event
  EcAioEvent event = ecaio_event_create();
  
  self->aio = aio;
  
  ecaio_event_setCallback(event, self, ecaio_proc_onDestroy);
  
  res = ecaio_event_assign (&event, aio, &(self->eventh), err);
  
  // thread part
  self->thread = ecthread_new ();
  ecthread_start(self->thread, ecaio_proc_thread, self);
  
  *pself = NULL;
  return res;
}

//*****************************************************************************

#elif defined __BSD_KEVENT

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

EcAioProc ecaio_proc_create (uint64_t pid)
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
#include "ecaio_event.h"

//-----------------------------------------------------------------------------

struct EcAioProc_s
{
  
  void* ptr;
  
  fct_ecaio_context_destroy onDestroy;
  
  fct_ecaio_context_onNotify onNotify;
  
  int pid;
  
  EcThread thread;
  
  EcAio aio;
  
  void* eventh;
  
};

//-----------------------------------------------------------------------------

static int __STDCALL ecaio_proc_thread (void* ptr)
{
  EcAioProc self = ptr;
  int res;
  
  waitpid (self->pid, &res, 0);

  {
    EcErr err = ecerr_create ();
    
    ecaio_triggerENode (self->aio, self->eventh, err);
    
    ecerr_destroy (&err);
  }

  return 0;
}

//-----------------------------------------------------------------------------

static int __STDCALL ecaio_proc_onNotify (void* ptr, int action)
{
  EcAioProc self = ptr;
  
  if (self->onNotify)
  {
    self->onNotify (self->ptr, 0);
  }
 
  return ENTC_AIO_CODE_ONCE;
}

//-----------------------------------------------------------------------------

static void __STDCALL ecaio_proc_onDestroy (void* ptr)
{
  EcAioProc self = ptr;
  
  ecthread_cancel (self->thread);
  
  if (self->onDestroy)
  {
    self->onDestroy (self->ptr);
  }
  
  ecthread_join (self->thread);
  
  ecthread_delete (&(self->thread));
  
  ENTC_DEL(&self, struct EcAioProc_s);
}

//-----------------------------------------------------------------------------

EcAioProc ecaio_proc_create (uint64_t pid)
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
  int res;
  EcAioProc self = *pself;
  
  // register a term event
  EcAioEvent event = ecaio_event_create();
  
  self->aio = aio;
  
  ecaio_event_setCallback (event, self, ecaio_proc_onNotify, ecaio_proc_onDestroy);
  
  res = ecaio_event_assign (&event, aio, &(self->eventh), err);
  
  // thread part
  self->thread = ecthread_new ();
  ecthread_start(self->thread, ecaio_proc_thread, self);
  
  *pself = NULL;
  return res;
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

