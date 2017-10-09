#include "ecaio_file.h"

// entc includes
#include "utils/eclogger.h"

#define READ_MAX_BUFFER 1024
#define ENTC_HANDLE long

//*****************************************************************************

#if defined __MS_IOCP

#include <windows.h>

//*****************************************************************************

struct EcAioFileReader_s
{
  
  HANDLE handle;
  
  char buffer [READ_MAX_BUFFER];
  
  fct_ecaio_context_onRead onRead;
  
  fct_ecaio_context_destroy destroy;
  
  void* ptr;
  
};

//-----------------------------------------------------------------------------

int ecaio_filereader_read (EcAioFileReader self, EcAioContext ctx)
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

static int __stdcall ecaio_filereader_fct_process (void* ptr, EcAioContext ctx, unsigned long len, unsigned long opt)
{
  EcAioFileReader self = ptr;
  
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

EcAioFileReader ecaio_filereader_create (void* handle)
{
  EcAioFileReader self = ENTC_NEW (struct EcAioFileReader_s);
  
  self->handle = handle;
  
  self->ptr = NULL;
  self->onRead = NULL;
  self->destroy = NULL;
  
  return self;
}

//-----------------------------------------------------------------------------

static void __stdcall ecaio_filereader_fct_destroy (void* ptr)
{
  EcAioFileReader self = ptr;
  
  if (self->destroy)
  {
    self->destroy (self->ptr);
  }
  
  ENTC_DEL (&self, struct EcAioFileReader_s);
}

//-----------------------------------------------------------------------------

int ecaio_filereader_assign (EcAioFileReader* pself, EcAio aio, EcErr err)
{
  EcAioFileReader self = *pself;
  
  {
    // link file handle with io port
    int res = ecaio_append (aio, self->handle, NULL, err);
    if (res)
    {
      return res;
    }
  }
  
  /*
   if (self->onInit)
   {
   int res = self->onInit (self->ptr, err);
   if (res)
   {
   return res;
   }
   }
   */
  
  {
    // create a async context
    EcAioContext ctx = ecaio_context_create ();
    
    // override callbacks
    ecaio_context_setCallbacks (ctx, self, ecaio_filereader_fct_process, ecaio_filereader_fct_destroy);
    
    // assign this and the context to the async system
    ecaio_filereader_read (self, ctx);
  }
  
  *pself = NULL;
  return ENTC_ERR_NONE;
}

//-----------------------------------------------------------------------------

void ecaio_filereader_setCallback(EcAioFileReader self, void* ptr, fct_ecaio_context_onInit onInit, fct_ecaio_context_onRead onRead, fct_ecaio_context_destroy destroy)
{
  self->ptr = ptr;
  self->onRead = onRead;
  self->destroy = destroy;
}

//=============================================================================

struct EcAioFileWriter_s
{
  HANDLE handle;
  
  fct_ecaio_context_destroy destroy;
  
  void* ptr;
  
  unsigned long written;
  
  unsigned long offset;
  
  EcBuffer buf;
  
};

//-----------------------------------------------------------------------------

int ecaio_filewriter_write (EcAioFileWriter self, EcAioContext ctx)
{
  DWORD written;
  DWORD res;
  
  //eclogger_fmt (LL_TRACE, "Q6_AIO", "write file", "starting");
  
  if (self->buf == NULL)
  {
    printf ("filewriter error -> buffer not set\r\n");
    return ENTC_AIO_CODE_DONE;
  }
  
  ecaio_context_appendOverlappedOffset (ctx, self->written);
  
  res = WriteFile (self->handle, self->buf->buffer + self->written, self->buf->size - self->written, &written, (LPOVERLAPPED)ecaio_context_getOverlapped (ctx));
  if (res == 0)
  {
    DWORD lastError = GetLastError ();
    if (lastError != ERROR_IO_PENDING)
    {
      EcErr err = ecerr_create ();

      ecerr_formatErrorOS (err, ENTC_LVL_ERROR, lastError);
      
      printf ("filewriter error -> %s\r\n", err->text);
      
	  ecerr_destroy (&err);
	  
	  return ENTC_AIO_CODE_DONE;
    }
    
  }
  
  //eclogger_fmt (LL_TRACE, "Q6_AIO", "write file", "wrote data %s", res);
  
  //printf ("filewriter continue\r\n");
  
  return ENTC_AIO_CODE_CONTINUE;
}

//-----------------------------------------------------------------------------

static int __stdcall ecaio_filewriter_fct_process (void* ptr, EcAioContext ctx, unsigned long len, unsigned long opt)
{
  EcAioFileWriter self = ptr;
  
  self->written += len;
  
  //printf ("filewriter %u numOfBytes from %u\r\n", len, self->buf->size);
  
  if (self->written < self->buf->size)
  {
    return ecaio_filewriter_write (self, ctx);
  }
  else
  {
    //printf ("filewriter done\r\n");
    return ENTC_AIO_CODE_DONE;
  }
}

//-----------------------------------------------------------------------------

static void __stdcall ecaio_filewriter_fct_destroy (void* ptr)
{
  EcAioFileWriter self = ptr;
  
  if (self->destroy)
  {
    self->destroy (self->ptr);
  }
  
  if (self->buf)
  {
    ecbuf_destroy (&(self->buf));
  }
  
  ENTC_DEL (&self, struct EcAioFileWriter_s);
}

//-----------------------------------------------------------------------------

EcAioFileWriter ecaio_filewriter_create (void* handle)
{
  EcAioFileWriter self = ENTC_NEW(struct EcAioFileWriter_s);
  
  self->handle = handle;
  self->written = 0;
  
  // buffer
  self->buf = NULL;
  
  // callbacks
  self->destroy = NULL;
  self->ptr = NULL;
  
  return self;
}

//-----------------------------------------------------------------------------

int ecaio_filewriter_assign (EcAioFileWriter* pself)
{
  EcAioFileWriter self = *pself;
  
  // create a async context
  EcAioContext ctx = ecaio_context_create ();
  
  // override callbacks
  ecaio_context_setCallbacks (ctx, self, ecaio_filewriter_fct_process, ecaio_filewriter_fct_destroy);
  
  //eclogger_fmt (LL_TRACE, "Q6_AIO", "write file", "assign");
  
  // assign this and the context to the async system
  ecaio_filewriter_write (self, ctx); // == OVL_PROCESS_CODE_CONTINUE;
  
  *pself = NULL;
  return ENTC_ERR_NONE;
}

//-----------------------------------------------------------------------------

void ecaio_filewriter_setBufferCP (EcAioFileWriter self, const char* buffer, unsigned long size)
{
  if (self->buf)
  {
    ecbuf_destroy (&(self->buf));
  }
  
  self->buf = ecbuf_create_fromBuffer ((const unsigned char*)buffer, size);
}

//-----------------------------------------------------------------------------

void ecaio_filewriter_setBufferBT (EcAioFileWriter self, EcBuffer* pbuf)
{
  if (self->buf)
  {
    ecbuf_destroy (&(self->buf));
  }
  
  self->buf = *pbuf;
  *pbuf = NULL;
}

//*****************************************************************************

#else

//*****************************************************************************

#include <errno.h>
#include <unistd.h>

//-----------------------------------------------------------------------------

struct EcAioFileReader_s
{
  
  ENTC_HANDLE handle;
  
  char buffer [READ_MAX_BUFFER];
  
  fct_ecaio_context_onRead onRead;
  
  fct_ecaio_context_destroy destroy;
  
  fct_ecaio_context_onInit onInit;
  
  void* ptr;
  
};

//-----------------------------------------------------------------------------

EcAioFileReader ecaio_filereader_create (void* handle)
{
  EcAioFileReader self = ENTC_NEW (struct EcAioFileReader_s);
  
  self->handle = (ENTC_HANDLE)handle;
  
  self->onRead = NULL;
  self->destroy = NULL;
  self->onInit = NULL;
  self->ptr = NULL;
  
  return self;
}

//-----------------------------------------------------------------------------

static void __STDCALL ecaio_filereader_fct_destroy (void* ptr)
{
  EcAioFileReader self = (EcAioFileReader)ptr;
  
  //eclogger_fmt (LL_ERROR, "Q6_AIO", "freader", "destroy");
  
  if (self->destroy)
  {
    self->destroy (self->ptr);
  }
  
  ENTC_DEL (&self, struct EcAioFileReader_s);
}

//-----------------------------------------------------------------------------

static int __STDCALL ecaio_filereader_fct_process (void* ptr, EcAioContext ctx, unsigned long val1, unsigned long val2)
{
  EcAioFileReader self = (EcAioFileReader)ptr;
  
  while (TRUE)
  {
    //eclogger_fmt (LL_TRACE, "Q6_AIO", "freader", "try to read");
    
    int count = read (self->handle, self->buffer, READ_MAX_BUFFER);
    
    //eclogger_fmt (LL_TRACE, "Q6_AIO", "freader", "read %i bytes", count);
    
    if (count == -1)
    {
      if (errno != EAGAIN) // If errno == EAGAIN, that means we have read all
      {
        return ENTC_AIO_CODE_CONTINUE;
      }
      
      return ENTC_AIO_CODE_DONE;
    }
    else if (count == 0)
    {
      return ENTC_AIO_CODE_DONE;
    }
    
    
    if (self->onRead)
    {
      int res = self->onRead (self->ptr, (void*)self->handle, self->buffer, count);
      if (res)
      {
        return ENTC_AIO_CODE_DONE;
      }
    }
  }
  
  return ENTC_AIO_CODE_CONTINUE;
}

//-----------------------------------------------------------------------------

void ecaio_filereader_setCallback(EcAioFileReader self, void* ptr, fct_ecaio_context_onInit onInit, fct_ecaio_context_onRead onRead, fct_ecaio_context_destroy destroy)
{
  self->ptr = ptr;
  self->onInit = onInit;
  self->onRead = onRead;
  self->destroy = destroy;
}

//-----------------------------------------------------------------------------

int ecaio_filereader_assign (EcAioFileReader* pself, EcAio aio, EcErr err)
{
  int res = ENTC_ERR_NONE;
  EcAioFileReader self = *pself;
  
  // create a async context
  EcAioContext ctx = ecaio_context_create ();
  
  // override callbacks
  ecaio_context_setCallbacks (ctx, self, ecaio_filereader_fct_process, ecaio_filereader_fct_destroy);
  
  res = ecaio_append (aio, (void*)self->handle, ctx, err);
  if (res)
  {
    if (res == ENTC_ERR_NOT_SUPPORTED)
    {
      //eclogger_fmt (LL_TRACE, "Q6_AIO", "freader", err->text);
      
      // try to just create an event instead
      res = ecaio_addContextToEvent (aio, ctx, err);
    }
    else
    {
      ecaio_context_destroy(&ctx);
      return res;
    }
  }
  
  if (self->onInit)
  {
    res = self->onInit (self->ptr, err);
    if (res)
    {
      ecaio_context_destroy(&ctx);
    }
  }
  
  //eclogger_fmt (LL_TRACE, "Q6_AIO", "freader", "active");
  
  return res;
}

//=============================================================================

struct EcAioFileWriter_s
{
  ENTC_HANDLE handle;
  
  unsigned long written;
  
  unsigned long offset;
  
  EcBuffer buf;
  
};

//-----------------------------------------------------------------------------

EcAioFileWriter ecaio_filewriter_create (void* handle)
{
  EcAioFileWriter self = ENTC_NEW(struct EcAioFileWriter_s);
  
  self->handle = (ENTC_HANDLE)handle;
  self->buf = NULL;
  self->offset = 0;
  self->written = 0;
  
  return self;
}

//-----------------------------------------------------------------------------

int ecaio_filewriter_assign (EcAioFileWriter* pself)
{
  EcAioFileWriter self = *pself;
  int del = 0;
  
  while (del < self->buf->size)
  {
    // important not to send to many bytes at once
    int maxBytes = ENTC_MIN (self->buf->size - del, 1024);
    
    // send the bytes to the FD
    int res = write (self->handle, (char*)self->buf->buffer + del, maxBytes);
    if (res < 0)
    {
      if( (errno != EWOULDBLOCK) && (errno != EINPROGRESS) && (errno != EAGAIN))
      {
        eclogger_fmt (LL_ERROR, "Q6_AIO", "write file", "can't write to pipe");
        break;
      }
      else
      {
        continue;
      }
    }
    else if (res == 0)
    {
      break;
    }
    else
    {
      del += res;
    }
  }
  
  if (self->buf)
  {
    ecbuf_destroy(&(self->buf));
  }
  
  ENTC_DEL(pself, struct EcAioFileWriter_s);
  return ENTC_ERR_NONE;
}

//-----------------------------------------------------------------------------

void ecaio_filewriter_setBufferCP (EcAioFileWriter self, const char* buffer, unsigned long size)
{
  if (self->buf)
  {
    ecbuf_destroy (&(self->buf));
  }
  
  self->buf = ecbuf_create_fromBuffer ((const unsigned char*)buffer, size);
}

//-----------------------------------------------------------------------------

void ecaio_filewriter_setBufferBT (EcAioFileWriter self, EcBuffer* pbuf)
{
  if (self->buf)
  {
    ecbuf_destroy (&(self->buf));
  }
  
  self->buf = *pbuf;
  *pbuf = NULL;
}

//-----------------------------------------------------------------------------

#endif
