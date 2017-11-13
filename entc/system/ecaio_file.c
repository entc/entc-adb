#include "ecaio_file.h"

// entc includes
#include "utils/eclogger.h"

#define READ_MAX_BUFFER 1024
#define ENTC_HANDLE long

//*****************************************************************************

#if defined __MS_IOCP

//*****************************************************************************

struct Q6AIOFileReader_s
{
  
  HANDLE handle;
  
  char buffer [READ_MAX_BUFFER];
  
  fct_aio_context_onRead onRead;
  
  fct_aio_context_destroy destroy;
  
  void* ptr;
  
};

//-----------------------------------------------------------------------------

int q6sys_aio_filereader_read (Q6AIOFileReader self, Q6AIOContext ctx)
{
  DWORD bytesRead;
  DWORD err;
  BOOL readstatus;
  
  readstatus = ReadFile (self->handle, self->buffer, READ_MAX_BUFFER, &bytesRead, (LPOVERLAPPED)ctx->overlapped);
  if (readstatus > 0)
  {
    return OVL_PROCESS_CODE_CONTINUE;
  }
  
  err = GetLastError ();
  if (err != ERROR_IO_PENDING)
  {
    return OVL_PROCESS_CODE_DONE;
  }
  
  return OVL_PROCESS_CODE_CONTINUE;
}

//-----------------------------------------------------------------------------

static int __stdcall q6sys_aio_filereader_fct_process (void* ptr, Q6AIOContext ctx, unsigned long len, unsigned long opt)
{
  Q6AIOFileReader self = (Q6AIOFileReader)ptr;
  
  if (len == 0)
  {
    return OVL_PROCESS_CODE_DONE;
  }
  
  ctx->overlapped->Offset += len;
  
  //std::cout << "RECEIVED BYTES: " << len << std::endl;
  
  if (self->onRead)
  {
    self->onRead (self->ptr, self->handle, self->buffer, len);
  }
  
  return q6sys_aio_filereader_read (self, ctx);
}

//-----------------------------------------------------------------------------

Q6AIOFileReader q6sys_aio_filereader_create (void* handle)
{
  Q6AIOFileReader self = ENTC_NEW (struct Q6AIOFileReader_s);
  
  self->handle = handle;
  
  self->ptr = NULL;
  self->onRead = NULL;
  self->destroy = NULL;
  
  return self;
}

//-----------------------------------------------------------------------------

static void __stdcall q6sys_aio_filereader_fct_destroy (void* ptr)
{
  Q6AIOFileReader self = (Q6AIOFileReader)ptr;
  
  if (self->destroy)
  {
    self->destroy (self->ptr);
  }
  
  ENTC_DEL (&self, struct Q6AIOFileReader_s);
}

//-----------------------------------------------------------------------------

int q6sys_aio_filereader_assign (Q6AIOFileReader* pself, Q6AIO aio, Q6Err err)
{
  Q6AIOFileReader self = *pself;
  
  {
    // link file handle with io port
    int res = q6sys_aio_append (aio, self->handle, NULL, err);
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
    Q6AIOContext ctx = q6sys_aio_context_create ();
    
    // override callbacks
    q6sys_aio_context_setCallbacks (ctx, self, q6sys_aio_filereader_fct_process, q6sys_aio_filereader_fct_destroy);
    
    // assign this and the context to the async system
    q6sys_aio_filereader_read (self, ctx);
  }
  
  *pself = NULL;
  return Q6ERR_NONE;
}

//-----------------------------------------------------------------------------

void q6sys_aio_filereader_setCallback(Q6AIOFileReader self, void* ptr, fct_aio_context_onInit onInit, fct_aio_context_onRead onRead, fct_aio_context_destroy destroy)
{
  self->ptr = ptr;
  self->onRead = onRead;
  self->destroy = destroy;
}

//=============================================================================

struct Q6AIOFileWriter_s
{
  HANDLE handle;
  
  fct_aio_context_destroy destroy;
  
  void* ptr;
  
  unsigned long written;
  
  unsigned long offset;
  
  EcBuffer buf;
  
};

//-----------------------------------------------------------------------------

int q6sys_aio_filewriter_write (Q6AIOFileWriter self, Q6AIOContext ctx)
{
  DWORD written;
  DWORD res;
  
  //eclogger_fmt (LL_TRACE, "Q6_AIO", "write file", "starting");
  
  if (self->buf == NULL)
  {
    printf ("filewriter error -> buffer not set\r\n");
    return OVL_PROCESS_CODE_DONE;
  }
  
  ctx->overlapped->Offset += self->written;
  
  res = WriteFile (self->handle, self->buf->buffer + self->written, self->buf->size - self->written, &written, (LPOVERLAPPED)ctx->overlapped);
  if (res == 0)
  {
    DWORD err = GetLastError ();
    if (err != ERROR_IO_PENDING)
    {
      struct Q6Err_s serr;
      q6err_formatErrorOS (&serr, Q6LVL_ERROR, err);
      
      printf ("filewriter error -> %s\r\n", serr.text);
      return OVL_PROCESS_CODE_DONE;
    }
    
  }
  
  //eclogger_fmt (LL_TRACE, "Q6_AIO", "write file", "wrote data %s", res);
  
  //printf ("filewriter continue\r\n");
  
  return OVL_PROCESS_CODE_CONTINUE;
}

//-----------------------------------------------------------------------------

static int __stdcall q6sys_aio_filewriter_fct_process (void* ptr, Q6AIOContext ctx, unsigned long len, unsigned long opt)
{
  Q6AIOFileWriter self = (Q6AIOFileWriter)ptr;
  
  self->written += len;
  
  //printf ("filewriter %u numOfBytes from %u\r\n", len, self->buf->size);
  
  if (self->written < self->buf->size)
  {
    return q6sys_aio_filewriter_write (self, ctx);
  }
  else
  {
    //printf ("filewriter done\r\n");
    return OVL_PROCESS_CODE_DONE;
  }
}

//-----------------------------------------------------------------------------

static void __stdcall q6sys_aio_filewriter_fct_destroy (void* ptr)
{
  Q6AIOFileWriter self = (Q6AIOFileWriter)ptr;
  
  if (self->destroy)
  {
    self->destroy (self->ptr);
  }
  
  if (self->buf)
  {
    ecbuf_destroy (&(self->buf));
  }
  
  ENTC_DEL (&self, struct Q6AIOFileWriter_s);
}

//-----------------------------------------------------------------------------

Q6AIOFileWriter q6sys_aio_filewriter_create (void* handle)
{
  Q6AIOFileWriter self = ENTC_NEW(struct Q6AIOFileWriter_s);
  
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

int q6sys_aio_filewriter_assign (Q6AIOFileWriter* pself)
{
  Q6AIOFileWriter self = *pself;
  
  // create a async context
  Q6AIOContext ctx = q6sys_aio_context_create ();
  
  // override callbacks
  q6sys_aio_context_setCallbacks (ctx, self, q6sys_aio_filewriter_fct_process, q6sys_aio_filewriter_fct_destroy);
  
  //eclogger_fmt (LL_TRACE, "Q6_AIO", "write file", "assign");
  
  // assign this and the context to the async system
  q6sys_aio_filewriter_write (self, ctx); // == OVL_PROCESS_CODE_CONTINUE;
  
  *pself = NULL;
  return Q6ERR_NONE;
}

//-----------------------------------------------------------------------------

void q6sys_aio_filewriter_setBufferCP (Q6AIOFileWriter self, const char* buffer, unsigned long size)
{
  if (self->buf)
  {
    ecbuf_destroy (&(self->buf));
  }
  
  self->buf = ecbuf_create_fromBuffer ((const unsigned char*)buffer, size);
}

//-----------------------------------------------------------------------------

void q6sys_aio_filewriter_setBufferBT (Q6AIOFileWriter self, EcBuffer* pbuf)
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
