#include "ecaio_event.h"

#include "system/macros.h"

//-----------------------------------------------------------------------------

struct EcAioEvent_s
{
  
  fct_ecaio_context_destroy onDestroy;
  
  fct_ecaio_context_onNotify onNotify;
  
  void* ptr;
  
};

//-----------------------------------------------------------------------------

EcAioEvent ecaio_event_create (void)
{
  EcAioEvent self = ENTC_NEW(struct EcAioEvent_s);
  
  self->onDestroy = NULL;
  self->ptr = NULL;
  
  return self;
}

//-----------------------------------------------------------------------------

void ecaio_event_destroy (EcAioEvent* pself)
{
  ENTC_DEL (pself, struct EcAioEvent_s);
}

//-----------------------------------------------------------------------------

static int __STDCALL ecaio_event_onProcess (void* ptr, EcAioContext ctx, unsigned long flags, unsigned long filter)
{
  EcAioEvent self = ptr;
  
  if (self->onNotify)
  {
    self->onNotify (self->ptr, flags);
  }
  
  return ENTC_AIO_CODE_ONCE;
}

//-----------------------------------------------------------------------------

static void __STDCALL ecaio_event_onDestroy (void* ptr)
{
  EcAioEvent self = ptr;
  
  if (self->onDestroy)
  {
    self->onDestroy (self->ptr);
  }
  
  ecaio_event_destroy (&self);
}

//-----------------------------------------------------------------------------

int ecaio_event_assign (EcAioEvent* pself, EcAio aio, void** eventh, EcErr err)
{
  int res = ENTC_ERR_NONE;
  EcAioEvent self = *pself;
  
  // create a async context
  EcAioContext ctx = ecaio_context_create ();
  
  // override callbacks
  ecaio_context_setCallbacks (ctx, self, ecaio_event_onProcess, ecaio_event_onDestroy);
  
  res = ecaio_appendENode (aio, ctx, eventh, err);
  if (res)
  {
    ecaio_event_destroy (pself);
    return res;
  }
  
  *pself = NULL;
  return res;
}

//-----------------------------------------------------------------------------

void ecaio_event_setCallback (EcAioEvent self, void* ptr, fct_ecaio_context_onNotify onNotify, fct_ecaio_context_destroy onDestroy)
{
  self->onDestroy = onDestroy;
  self->onNotify = onNotify;
  self->ptr = ptr;
}

//-----------------------------------------------------------------------------
