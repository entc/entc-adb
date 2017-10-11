#include "ecaio_msg.h"

#if defined __MS_IOCP && defined __MS_MQRT

//*****************************************************************************

#include <windows.h>
#include <mq.h>
#include <stdio.h>

#pragma comment(lib, "Mqrt.lib")

struct EcMsgChannel_s
{
	QUEUEHANDLE hQueue;
};

//-----------------------------------------------------------------------------

EcMsgChannel ecmsg_channel_create ()
{
  EcMsgChannel self = ENTC_NEW(struct EcMsgChannel_s);

  self->hQueue = NULL;

  return self;
}

//-----------------------------------------------------------------------------

void ecmsg_channel_destroy (EcMsgChannel* pself)
{
  EcMsgChannel self = *pself;

  if (self->hQueue)
  {
    MQCloseQueue(self->hQueue);
  }

  ENTC_DEL (pself, struct EcMsgChannel_s);
}

//-----------------------------------------------------------------------------

int ecmsg_channel_init (EcMsgChannel self, const EcString name, EcErr err)
{
  HRESULT hr;

  wchar_t* wname = ecstr_utf8ToWide (name);

  hr = MQOpenQueue (wname, MQ_RECEIVE_ACCESS, MQ_DENY_NONE, &(self->hQueue));

  free(wname);

  if (FAILED(hr))  
  { 
    return ecerr_set (err, ENTC_LVL_FATAL, ENTC_ERR_OS_ERROR, "can't open Windows MQOpenQueue");
  } 

  return ENTC_ERR_NONE;
}

//-----------------------------------------------------------------------------

void* ecmsg_channel_handle (EcMsgChannel self)
{
  return (void*)self->hQueue;
}

//=============================================================================

struct EcAioMsgReader_s
{
  
  fct_ecaio_context_onRead onRead;
  
  fct_ecaio_context_destroy onDestroy;
  
  void* ptr;

  HANDLE hQueue;              // Queue handle  
    
  MQMSGPROPS* props;          // Message properties structure

};

//-----------------------------------------------------------------------------

EcAioMsgReader ecaio_msgreader_create (void* channelHandle)
{
  EcAioMsgReader self = ENTC_NEW (struct EcAioMsgReader_s);
  
  self->onRead = NULL;
  self->onDestroy = NULL;
  self->ptr = NULL;

  self->hQueue = channelHandle;

  self->props = ENTC_NEW(MQMSGPROPS); 

  self->props->aPropID = ENTC_NEW(MSGPROPID);
  self->props->aPropVar = ENTC_NEW(MQPROPVARIANT);
  self->props->aStatus = ENTC_NEW(HRESULT);
  
  return self;
}

//-----------------------------------------------------------------------------

int ecaio_msgreader_read (EcAioMsgReader self, EcAioContext ctx)
{
  HRESULT hr;
  
  self->props->aPropID[0] = PROPID_M_APPSPECIFIC;  
  self->props->aPropVar[0].vt = VT_UI4;  
  self->props->cProp = 0;  

  hr = MQReceiveMessage (self->hQueue, INFINITE, MQ_ACTION_RECEIVE, self->props, (LPOVERLAPPED)ecaio_context_getOverlapped(ctx), NULL, NULL, MQ_NO_TRANSACTION);
  if (FAILED(hr))  
  { 
    return ENTC_AIO_CODE_DONE;  
  }
  else
  {
    return ENTC_AIO_CODE_CONTINUE;
  }
}

//-----------------------------------------------------------------------------

static int __stdcall ecaio_msgreader_fct_process (void* ptr, EcAioContext ctx, unsigned long len, unsigned long opt)
{
  EcAioMsgReader self = ptr;
  
  if (len == 0)
  {
    return ENTC_AIO_CODE_DONE;
  }
    
  // TODO: read out the message

  if (self->onRead)
  {
	  //self->onRead (self->ptr, self->handle, self->pmsgprops->, len);
  }
  
  return ecaio_msgreader_read (self, ctx);
}

//-----------------------------------------------------------------------------

static void __stdcall ecaio_msgreader_fct_destroy (void* ptr)
{
  EcAioMsgReader self = ptr;
  
  if (self->onDestroy)
  {
    self->onDestroy (self->ptr);
  }
  
  ENTC_DEL (&self, struct EcAioMsgReader_s);
}

//-----------------------------------------------------------------------------

int ecaio_msgreader_assign (EcAioMsgReader* pself, EcAio aio, EcErr err)
{
  EcAioMsgReader self = *pself;
  
  {
    // link file handle with io port
	int res = ecaio_append (aio, self->hQueue, NULL, err);
    if (res)
    {
      return res;
    }
  }
  
  {
    // create a async context
    EcAioContext ctx = ecaio_context_create ();
    
    // override callbacks
    ecaio_context_setCallbacks (ctx, self, ecaio_msgreader_fct_process, ecaio_msgreader_fct_destroy);
    
    // assign this and the context to the async system
    ecaio_msgreader_read (self, ctx);
  }
  
  *pself = NULL;
  return ENTC_ERR_NONE;
}

//*****************************************************************************

#else

//*****************************************************************************

struct EcAioMsgReader_s
{
  
  fct_ecaio_context_onRead onRead;
  
  fct_ecaio_context_destroy onDestroy;
  
  void* ptr;
  
};

//-----------------------------------------------------------------------------




//*****************************************************************************

#endif

//*****************************************************************************

void ecaio_msgreader_setCallback (EcAioMsgReader self, void* ptr, fct_ecaio_context_onRead onRead, fct_ecaio_context_destroy onDestroy)
{
  self->ptr = ptr;
  self->onDestroy = onDestroy;
  self->onRead = onRead;
}

//-----------------------------------------------------------------------------

