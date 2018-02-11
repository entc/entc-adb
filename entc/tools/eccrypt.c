#include "eccrypt.h"
#include "tools/eclog.h"
#include "types/ecstream.h"

#include <openssl/aes.h>
#include <openssl/evp.h>
#include <openssl/err.h>
#include <openssl/ssl.h>


//----------------------------------------------------------------------------------------

EcString eccrypt_aes_getkey (const EcString secret, EcErr err)
{
  EcBuffer_s h;
  EcBuffer key;

  h.buffer = (unsigned char*)secret;
  h.size = ecstr_len(secret);
  
  key = ecbuf_sha_256 (&h, err);
  if (key == NULL)
  {
    return NULL;
  }
  
  return ecbuf_str(&key);
}

#if defined _WIN32 || defined _WIN64

//=============================================================================

struct EcEncryptAES_s
{
  
  HCRYPTPROV hProvider;
  
  EcBuffer buf;
  
};

//-----------------------------------------------------------------------------

EcEncryptAES ecencrypt_aes_create ()
{
  EcEncryptAES self = ENTC_NEW (struct EcEncryptAES_s);
  
  self->hProvider = NULL;
  self->buf = NULL;
  
  return self;
}

//-----------------------------------------------------------------------------

void ecencrypt_aes_destroy (EcEncryptAES* pself)
{
  EcEncryptAES self = *pself;
  
  if (self->hProvider)
  {
    CryptReleaseContext (self->hProvider, 0);
  }
  
  if (self->buf)
  {
    ecbuf_destroy (&(self->buf)); 
  }
    
  ENTC_DEL (pself, struct EcEncryptAES_s);
}

//-----------------------------------------------------------------------------

EcEncryptAES ecencrypt_aes_initialize (const EcString secret, EcErr err)
{
  EcEncryptAES self = ecencrypt_aes_create ();
  
  if (!CryptAcquireContext (&(self->hProvider), NULL, NULL, PROV_RSA_AES, CRYPT_VERIFYCONTEXT))
  {
    ecerr_lastErrorOS (err, ENTC_LVL_ERROR);
    
    // cleanup
    ecencrypt_aes_destroy (&self);
    
    return NULL;
  }
  
  return self;
}

//-----------------------------------------------------------------------------

EcBuffer ecencrypt_aes_update (EcEncryptAES self, EcBuffer source, EcErr err)
{  
  AesBlob128 aes_blob;
  
  aes_blob.header.bType = PLAINTEXTKEYBLOB;
  
  
  return self->buf;
}

//-----------------------------------------------------------------------------

EcBuffer ecencrypt_aes_finalize (EcEncryptAES self, EcErr err)
{
  return self->buf;
}

//=============================================================================

struct EcDecryptAES_s
{

	int dummy;

};

//-----------------------------------------------------------------------------

EcDecryptAES ecdecrypt_aes_initialize (const EcString secret, EcErr err)
{

}

//-----------------------------------------------------------------------------

EcBuffer ecdecrypt_aes_update (EcDecryptAES self, EcBuffer buffer, EcErr err)
{

}

//-----------------------------------------------------------------------------

EcBuffer ecdecrypt_aes_finalize (EcDecryptAES self, EcErr err)
{

}

//-----------------------------------------------------------------------------

void ecdecrypt_aes_destroy (EcDecryptAES* pself)
{

}

//-----------------------------------------------------------------------------

#elif defined __BSD_OS || defined __LINUX_OS

//----------------------------------------------------------------------------------------

int eccrypt_aes_handleError (EVP_CIPHER_CTX* ctx, EcErr err)
{
  unsigned long errCode;
  int res;
  EcStream stream = ecstream_create ();
  
  while ((errCode = ERR_get_error()))
  {
    ecstream_append_str (stream, ERR_error_string (errCode, NULL));
  }
  
  res = ecerr_set (err, ENTC_LVL_ERROR, ENTC_ERR_PROCESS_FAILED, ecstream_get (stream));
  
  ecstream_destroy (&stream);
  
  return res;
}

//=============================================================================

struct EcEncryptAES_s
{
  
  EVP_CIPHER_CTX ctx;
  
  EcBuffer buf;
  
  int blocksize;
  
  int64_t lenTotal;
  
};

//-----------------------------------------------------------------------------

EcEncryptAES ecencrypt_aes_create (void)
{
  EcEncryptAES self = ENTC_NEW(struct EcEncryptAES_s);
  
  EVP_CIPHER_CTX_init (&(self->ctx));
  
  self->blocksize = 0;
  self->lenTotal = 0;
  
  self->buf = NULL;
  
  return self;
}

//-----------------------------------------------------------------------------

void ecencrypt_aes_destroy (EcEncryptAES* pself)
{
  EcEncryptAES self = *pself;
  
  EVP_CIPHER_CTX_cleanup (&(self->ctx));
  
  if (self->buf)
  {
    ecbuf_destroy (&(self->buf));
  }
  
  ENTC_DEL(pself, struct EcEncryptAES_s);
}

//-----------------------------------------------------------------------------

EcEncryptAES ecencrypt_aes_initialize (const EcString secret, EcErr err)
{
  int res;
  EcEncryptAES self = ecencrypt_aes_create ();
  
  EcString key = eccrypt_aes_getkey (secret, err);
  if (key == NULL)
  {
    ecencrypt_aes_destroy (&self);
    
    return NULL;
  }
  
  res = EVP_EncryptInit (&(self->ctx), EVP_aes_256_cbc(), (unsigned char*)key, NULL);
  
  ecstr_delete (&key);
  
  if (res == 0)
  {
    eccrypt_aes_handleError (&(self->ctx), err);
    
    ecencrypt_aes_destroy (&self);
    
    return NULL;
  }
  
  // check for the blocksize
  self->blocksize = EVP_CIPHER_CTX_block_size(&(self->ctx));

  return self;
}

//-----------------------------------------------------------------------------

EcBuffer ecencrypt_aes_update (EcEncryptAES self, EcBuffer source, EcErr err)
{
  // check buffer size
  if (self->buf)
  {
    ecbuf_resize (self->buf, source->size + self->blocksize);
  }
  else
  {
    self->buf = ecbuf_create (source->size + self->blocksize);
  }
  
  int lenLast;
  
  if (EVP_EncryptUpdate(&(self->ctx), self->buf->buffer, &lenLast, source->buffer, source->size) == 0)
  {
    eccrypt_aes_handleError (&(self->ctx), err);
    return NULL;
  }

  self->buf->size = lenLast;
  self->lenTotal += lenLast;
  
  return self->buf;
}

//-----------------------------------------------------------------------------

EcBuffer ecencrypt_aes_finalize (EcEncryptAES self, EcErr err)
{
  // check buffer size
  if (self->buf)
  {
    ecbuf_resize (self->buf, self->blocksize);
  }
  else
  {
    self->buf = ecbuf_create (self->blocksize);
  }

  int lenLast;

  if (EVP_EncryptFinal_ex(&(self->ctx), self->buf->buffer, &lenLast) == 0)
  {
    eccrypt_aes_handleError (&(self->ctx), err);
    return NULL;
  }

  self->buf->size = lenLast;
  self->lenTotal += lenLast;

  return self->buf;
}

//=============================================================================

struct EcDecryptAES_s
{
  EVP_CIPHER_CTX ctx;
  
  EcBuffer buf;
  
  int blocksize;
  
  int64_t lenTotal;

};

//-----------------------------------------------------------------------------

EcDecryptAES ecdecrypt_aes_create (void)
{
  EcDecryptAES self = ENTC_NEW(struct EcDecryptAES_s);
  
  EVP_CIPHER_CTX_init (&(self->ctx));

  self->blocksize = 0;
  self->lenTotal = 0;
  
  self->buf = NULL;
  
  return self;
}

//-----------------------------------------------------------------------------

void ecdecrypt_aes_destroy (EcDecryptAES* pself)
{
  EcDecryptAES self = *pself;
  
  EVP_CIPHER_CTX_cleanup (&(self->ctx));
  
  if (self->buf)
  {
    ecbuf_destroy (&(self->buf));
  }

  ENTC_DEL(pself, struct EcDecryptAES_s);
}

//-----------------------------------------------------------------------------

EcDecryptAES ecdecrypt_aes_initialize (const EcString secret, EcErr err)
{
  int res;
  EcDecryptAES self = ecdecrypt_aes_create ();
  
  EcString key = eccrypt_aes_getkey (secret, err);
  if (key == NULL)
  {
    ecdecrypt_aes_destroy (&self);
    
    return NULL;
  }
  
  res = EVP_DecryptInit (&(self->ctx), EVP_aes_256_cbc(), (unsigned char*)key, NULL);
  
  ecstr_delete (&key);
  
  if (res == 0)
  {
    eccrypt_aes_handleError (&(self->ctx), err);
    
    ecdecrypt_aes_destroy (&self);

    return NULL;
  }
  
  // check for the blocksize
  self->blocksize = EVP_CIPHER_CTX_block_size(&(self->ctx));
  
  return self;
}

//-----------------------------------------------------------------------------

EcBuffer ecdecrypt_aes_update (EcDecryptAES self, EcBuffer source, EcErr err)
{
  // check buffer size
  if (self->buf)
  {
    ecbuf_resize (self->buf, source->size + self->blocksize);
  }
  else
  {
    self->buf = ecbuf_create (source->size + self->blocksize);
  }
  
  int lenLast;
  
  if (EVP_DecryptUpdate (&(self->ctx), self->buf->buffer, &lenLast, source->buffer, source->size) == 0)
  {
    eccrypt_aes_handleError (&(self->ctx), err);
    return NULL;
  }
  
  self->buf->size = lenLast;
  self->lenTotal += lenLast;
  
  return self->buf;
}

//-----------------------------------------------------------------------------

EcBuffer ecdecrypt_aes_finalize (EcDecryptAES self, EcErr err)
{
  // check buffer size
  if (self->buf)
  {
    ecbuf_resize (self->buf, self->blocksize);
  }
  else
  {
    self->buf = ecbuf_create (self->blocksize);
  }
  
  int lenLast;
  
  if (EVP_DecryptFinal_ex(&(self->ctx), self->buf->buffer, &lenLast) == 0)
  {
    eccrypt_aes_handleError (&(self->ctx), err);
    return NULL;
  }
  
  self->buf->size = lenLast;
  self->lenTotal += lenLast;
  
  return self->buf;
}

//-------------------------------------------------------------------------------------------------------

#endif

//-------------------------------------------------------------------------------------------------------

int ecencrypt_file_copy (EcEncryptAES aes, EcFileHandle fhIn, EcFileHandle fhOut)
{
  EcBuffer ben;
  EcBuffer bin = ecbuf_create (1024);
  int bytesRead;
  int bytesWritten;
  
  for (bytesRead = ecfh_readBuffer (fhIn, bin); bytesRead > 0; bytesRead = ecfh_readBuffer (fhIn, bin))
  {
    EcBuffer_s h;
    
    h.buffer = bin->buffer;
    h.size = bytesRead;
    
    ben = ecencrypt_aes_update (aes, &h, NULL);
    if (ben == NULL)
    {
      return ENTC_ERR_PROCESS_FAILED;
    }
    
    bytesWritten = ecfh_writeBuffer (fhOut, ben, ben->size);
    if (bytesWritten < 0)
    {
      eclog_fmt (LL_ERROR, "ENTC", "enc file", "can't write bytes");
      
      return ENTC_ERR_PROCESS_FAILED;
    }
  }
  
  ben = ecencrypt_aes_finalize (aes, NULL);
  if (ben == NULL)
  {
    return ENTC_ERR_PROCESS_FAILED;
  }
  
  ecfh_writeBuffer (fhOut, ben, ben->size);
  
  return ENTC_ERR_NONE;
}

//-----------------------------------------------------------------------------

int ecencrypt_file (const EcString source, const EcString dest, const EcString secret, EcErr err)
{
  EcEncryptAES aes;
  EcFileHandle fhIn;
  EcFileHandle fhOut;
  
  int res;
  
  aes = ecencrypt_aes_initialize (secret, err);
  if (aes == NULL)
  {
    return ENTC_ERR_PROCESS_FAILED;
  }
  
  fhIn = ecfh_open (source, O_RDONLY);
  if (fhIn == NULL)
  {
    ecencrypt_aes_destroy (&aes);
    
    eclog_fmt (LL_WARN, "ENTC", "enc file", "can't open original file '%s'", source);
    
    return ENTC_ERR_PROCESS_FAILED;
  }
  
  fhOut = ecfh_open (dest, O_CREAT | O_TRUNC | O_WRONLY);
  if (fhOut == NULL)
  {
    ecfh_close (&fhIn);
    
    ecencrypt_aes_destroy (&aes);
    
    eclog_fmt (LL_WARN, "ENTC", "enc file", "can't open dest file '%s'", dest);
    
    return ENTC_ERR_PROCESS_FAILED;
  }
  
  eclog_fmt (LL_TRACE, "ENTC", "end file", "encrypt file");
  
  res = ecencrypt_file_copy (aes, fhIn, fhOut);
  
  ecfh_close (&fhIn);
  ecfh_close (&fhOut);
  
  ecencrypt_aes_destroy (&aes);
  
  return res;
}

//-------------------------------------------------------------------------------------------------------

int ecdecrypt_file_copy (EcDecryptAES aes, EcFileHandle fhIn, EcFileHandle fhOut)
{
  EcBuffer ben;
  EcBuffer bin = ecbuf_create (1024);
  int bytesRead;
  int bytesWritten;
  
  for (bytesRead = ecfh_readBuffer (fhIn, bin); bytesRead > 0; bytesRead = ecfh_readBuffer (fhIn, bin))
  {
    EcBuffer_s h;
    
    h.buffer = bin->buffer;
    h.size = bytesRead;
    
    ben = ecdecrypt_aes_update (aes, &h, NULL);
    if (ben == NULL)
    {
      return ENTC_ERR_PROCESS_FAILED;
    }
    
    bytesWritten = ecfh_writeBuffer (fhOut, ben, ben->size);
    if (bytesWritten < 0)
    {
      eclog_fmt (LL_ERROR, "ENTC", "dec file", "can't write bytes");
      
      return ENTC_ERR_PROCESS_FAILED;
    }
  }
  
  ben = ecdecrypt_aes_finalize (aes, NULL);
  if (ben == NULL)
  {
    return ENTC_ERR_PROCESS_FAILED;
  }
  
  ecfh_writeBuffer (fhOut, ben, ben->size);
  
  return ENTC_ERR_NONE;
}

//-----------------------------------------------------------------------------

int ecdecrypt_file (const EcString source, const EcString dest, const EcString secret, EcErr err)
{
  EcDecryptAES aes;
  EcFileHandle fhIn;
  EcFileHandle fhOut;
  
  int res;
  
  aes = ecdecrypt_aes_initialize (secret, err);
  if (aes == NULL)
  {
    return ENTC_ERR_PROCESS_FAILED;
  }
  
  fhIn = ecfh_open (source, O_RDONLY);
  if (fhIn == NULL)
  {
    ecdecrypt_aes_destroy (&aes);
    
    eclog_fmt (LL_WARN, "ENTC", "dec file", "can't open original file '%s'", source);
    
    return ENTC_ERR_PROCESS_FAILED;
  }
  
  fhOut = ecfh_open (dest, O_CREAT | O_TRUNC | O_WRONLY);
  if (fhOut == NULL)
  {
    ecfh_close (&fhIn);
    
    ecdecrypt_aes_destroy (&aes);
    
    eclog_fmt (LL_WARN, "ENTC", "dec file", "can't open dest file '%s'", dest);
    
    return ENTC_ERR_PROCESS_FAILED;
  }
  
  eclog_fmt (LL_TRACE, "ENTC", "dec file", "decrypt file");
  
  res = ecdecrypt_file_copy (aes, fhIn, fhOut);
  
  ecfh_close (&fhIn);
  ecfh_close (&fhOut);
  
  ecdecrypt_aes_destroy (&aes);
  
  return res;
}

//-----------------------------------------------------------------------------
