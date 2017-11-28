#include "ecaio_msg.h"

#if defined __MS_IOCP

//*****************************************************************************

#include <windows.h>
#include <stdio.h>

//-----------------------------------------------------------------------------

struct EcAioMsgReader_s
{
  
  fct_ecaio_context_onRead onRead;
  
  fct_ecaio_context_destroy onDestroy;
  
  void* ptr;
  
};

//-----------------------------------------------------------------------------

EcAioMsgReader ecaio_msgreader_create (uint64_t channel)
{
  EcAioMsgReader self = ENTC_NEW (struct EcAioMsgReader_s);
  
  self->onRead = NULL;
  self->destroy = NULL;
  self->ptr = NULL;
  
  return self;
}

//-----------------------------------------------------------------------------

int ecaio_msgreader_read (EcAioMsgReader self, EcAioContext ctx)
{
  DWORD bytesRead;
  DWORD err;
  BOOL readstatus;
  
  readstatus = ReadFile (self->handle, self->buffer, READ_MAX_BUFFER, &bytesRead, (LPOVERLAPPED)ecaio_context_getOverlapped(ctx));
  if (readstatus > 0)
  {
    return ENTC_AIO_CODE_CONTINUE;
  }
  
  err = GetLastError ();
  if (err != ERROR_IO_PENDING)
  {
    return ENTC_AIO_CODE_DONE;
  }
  
  return ENTC_AIO_CODE_CONTINUE;
}

//-----------------------------------------------------------------------------

static int __stdcall ecaio_msgreader_fct_process (void* ptr, EcAioContext ctx, unsigned long len, unsigned long opt)
{
  EcAioMsgReader self = ptr;
  
  if (len == 0)
  {
    return ENTC_AIO_CODE_DONE;
  }
  
  ecaio_context_appendOverlappedOffset (ctx, len);
  
  //std::cout << "RECEIVED BYTES: " << len << std::endl;
  
  if (self->onRead)
  {
    self->onRead (self->ptr, self->handle, self->buffer, len);
  }
  
  return ecaio_filereader_read (self, ctx);
}

//-----------------------------------------------------------------------------

static void __stdcall ecaio_msgreader_fct_destroy (void* ptr)
{
  EcAioMsgReader self = ptr;
  
  if (self->destroy)
  {
    self->destroy (self->ptr);
  }
  
  ENTC_DEL (&self, struct EcAioMsgReader_s);
}

//-----------------------------------------------------------------------------

int ecaio_msgreader_assign (EcAioMsgReader*, EcAio aio, EcErr err)
{
  EcAioMsgReader self = *pself;
  
  {
    // link file handle with io port
    int res = ecaio_append (aio, self->handle, NULL, err);
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

