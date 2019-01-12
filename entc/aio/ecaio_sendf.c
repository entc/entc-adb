#include "ecaio_sendf.h"

#include "system/ecfile.h"
#include "aio/ecaio_file.h"
#include "tools/eclog.h"
#include "tools/eccrypt.h"
#include "tools/eccode.h"

#if defined __WIN_OS
#include <windows.h>
#endif

//=============================================================================

struct EcAioSendFile_s
{
  
  EcString file;
  
  EcString name;
  
  EcDecryptAES aes;         // input decryption
  
  EcBase64Encode base64;    // output format encoding
 
  fct_ecaio_sfile_onInit onInit;
  void* ptr;
  
  EcFileHandle fh;
  
  EcRefCountedSocket refSocket;
  
};

//-----------------------------------------------------------------------------

EcAioSendFile ecaio_sendfile_create (const EcString file, const EcString name, EcRefCountedSocket refSocket, void* ptr, fct_ecaio_sfile_onInit onInit)
{
  EcAioSendFile self = ENTC_NEW (struct EcAioSendFile_s);
  
  self->file = ecstr_copy (file);
  self->name = ecstr_copy (name);
  
  self->onInit = onInit;
  self->ptr = ptr;
  
  self->refSocket = ecrefsocket_clone (refSocket);
  
  self->aes = NULL;
  self->base64 = NULL;
  
  return self;
}

//-----------------------------------------------------------------------------

void ecaio_sendfile_destroy (EcAioSendFile* pself)
{
  EcAioSendFile self = *pself;

  if (self->aes)
  {
    ecdecrypt_aes_destroy (&(self->aes));
  }
  
  if (self->base64)
  {
    eccode_base64_encode_destroy (&(self->base64));
  }
  
  ecstr_delete (&(self->file));
  ecstr_delete (&(self->name));
  
  if (self->fh)
  {
#if defined __WIN_OS
    CloseHandle (self->fh);
#else
    ecfh_close(&(self->fh));
#endif
  }
  
  ecrefsocket_decrease (&(self->refSocket));
  
  ENTC_DEL (pself, struct EcAioSendFile_s);
}

//-----------------------------------------------------------------------------

int ecaio_sendfile_setSecret (EcAioSendFile self, const EcString secret, unsigned int sectype, EcErr err)
{
  // initialize the decryption
  self->aes = ecdecrypt_aes_create (secret, sectype, 0);
  
  return ENTC_ERR_NONE;
}

//-----------------------------------------------------------------------------

void ecaio_sendfile_setBase64 (EcAioSendFile self, int base64)
{
  if (base64)
  {
    self->base64 = eccode_base64_encode_create ();
  }
}

//-----------------------------------------------------------------------------

int ecaio_sendfile_setBuffer_update (EcAioSendFile self, EcAioSocketWriter writer, EcBuffer buf, EcErr err)
{
  if (self->base64)
  {
    // create a buffer for encode the base64
    EcBuffer h = ecbuf_create (eccode_base64_encode_size (buf->size));
    
    // correct buffer length
    h->size = eccode_base64_encode_update (self->base64, h, buf, err);
    
    // transfer ownership
    ecaio_socketwriter_setBufferBT (writer, &h);
  }
  else
  {
    // we need a copy here :-(
    ecaio_socketwriter_setBufferCP (writer, (char*)buf->buffer, buf->size);
  }
  
  return ENTC_ERR_NONE;
}

//-----------------------------------------------------------------------------

int ecaio_sendfile_setBuffer_finalize (EcAioSendFile self, EcAioSocketWriter writer, EcBuffer buf, EcErr err)
{
  if (self->base64)
  {
    // create a buffer for encode the base64 + extra length for finalization
    EcBuffer h = ecbuf_create (eccode_base64_encode_size (buf->size) + 20);
    
    // correct buffer length
    uint64_t offset = eccode_base64_encode_update (self->base64, h, buf, err);

    // fake buffer
    {
      EcBuffer_s h2;
      
      h2.buffer = h->buffer + offset;
      h2.size = h->size - offset;
      
      // correct buffer length
      h->size = eccode_base64_encode_finalize (self->base64, &h2, err) + offset;
    }
    
    // transfer ownership
    ecaio_socketwriter_setBufferBT (writer, &h);
  }
  else
  {
    // we need a copy here :-(
    ecaio_socketwriter_setBufferCP (writer, (char*)buf->buffer, buf->size);
  }
  
  return ENTC_ERR_NONE;
}

//-----------------------------------------------------------------------------

static void __STDCALL ecaio_sendfile_onDestroy (void* ptr)
{
  EcAioSendFile self = ptr;
  
  if (self->aes)
  {
    EcBuffer g;
    EcErr err = ecerr_create();
    
    EcAioSocketWriter writer;
    int res;
    
    writer = ecaio_socketwriter_create (self->refSocket);

    g = ecdecrypt_aes_finalize (self->aes, err);
    if (g)
    {
      //eclogger_fmt (LL_TRACE, "Q6_SOCK", "sfile", "wrote decrypted bytes %i", g->size);

      // we need a copy here :-(
      ecaio_sendfile_setBuffer_finalize (self, writer, g, err);
    }
    
    res = ecaio_socketwriter_assign (&writer, err);
    if (res)
    {
      eclog_fmt (LL_ERROR, "Q6_SOCK", "sfstream", "write to socket %i: %s", ecrefsocket_socket (self->refSocket), err->text);
    }
    
    ecerr_destroy(&err);
  }
  
  //eclogger_fmt (LL_TRACE, "Q6_SOCK", "sfile", "close");
  
  ecaio_sendfile_destroy (&self);
}

//-----------------------------------------------------------------------------

static int __STDCALL ecaio_sendfile_onRead (void* ptr, void* handle, const char* buffer, unsigned long len)
{
  EcAioSendFile self = ptr;
  
  EcAioSocketWriter writer;
  int res;
  EcErr err = ecerr_create();
  
  //eclogger_fmt (LL_TRACE, "Q6_SOCK", "sfile", "on read [%p]", self);
  
  writer = ecaio_socketwriter_create (self->refSocket);
 
  if (self->aes)
  {
    EcBuffer_s h;
    EcBuffer g;

    h.buffer = (unsigned char*)buffer;
    h.size = len;
    
    g = ecdecrypt_aes_update (self->aes, &h, err);
    if (g)
    {
      //eclogger_fmt (LL_TRACE, "Q6_SOCK", "sfile", "wrote decrypted bytes %i", g->size);
      
      ecaio_sendfile_setBuffer_update (self, writer, g, err);
    }
  }
  else
  {
    EcBuffer_s h;

    h.buffer = (unsigned char*)buffer;
    h.size = len;

    ecaio_sendfile_setBuffer_update (self, writer, &h, err);
  }
  
  res = ecaio_socketwriter_assign (&writer, err);
  if (res)
  {
    eclog_fmt (LL_ERROR, "Q6_SOCK", "sfstream", "write to socket %i: %s", ecrefsocket_socket (self->refSocket), err->text);
  }
  
  //eclogger_fmt (LL_TRACE, "Q6_SOCK", "sfile", "write done");
  
  ecerr_destroy(&err);
  
  return res;
}

//-----------------------------------------------------------------------------

int ecaio_sendfile_assign (EcAioSendFile* pself, EcAio aio, EcErr err)
{
  uint64_t fileSize;
  EcAioSendFile self = *pself;
  void* handle = NULL;
  
#if defined __WIN_OS
  
  // open file for reading
  self->fh = CreateFileA (self->file, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, NULL);
  if (self->fh == INVALID_HANDLE_VALUE)
  {
    ecaio_sendfile_destroy (pself);
    
    return ecerr_lastErrorOS (err, ENTC_LVL_ERROR);
  }
  
  fileSize = GetFileSize (self->fh, NULL);
  if (fileSize == INVALID_FILE_SIZE)
  {
    ecaio_sendfile_destroy (pself);
    
    return ecerr_lastErrorOS (err, ENTC_LVL_ERROR);
  }
  
  handle = self->fh;
  
#else
  
  self->fh = ecfh_open (self->file, O_RDONLY);
  if (self->fh == NULL)
  {
    ecaio_sendfile_destroy (pself);
    
    return ecerr_lastErrorOS (err, ENTC_LVL_ERROR);
  }
  
  fileSize = ecfh_size (self->fh);
  
  {
    // retrieve the file descriptor
    unsigned long fd = ecfh_fileno (self->fh);
    
    handle = (void*)fd;
  }
  
#endif
  
  if (self->onInit)
  {
    int res = self->onInit (self->ptr, self->refSocket, self->aes ? 0 : fileSize, self->file, self->name, err);
    if (res)
    {
      ecaio_sendfile_destroy (pself);
      
      return res;
    }
  }
  
  if (fileSize == 0)
  {
    ecaio_sendfile_destroy (pself);
    
    return ENTC_ERR_NONE;
  }
  
  {
    int res;
    
    EcAioFileReader freader = ecaio_filereader_create (handle);
    
    ecaio_filereader_setCallback (freader, self, NULL, ecaio_sendfile_onRead, ecaio_sendfile_onDestroy);
    
    res = ecaio_filereader_assign (&freader, aio, err);
    if (res)
    {
      return res;
    }
  }
  
  return ENTC_ERR_NONE;
}

//-----------------------------------------------------------------------------
