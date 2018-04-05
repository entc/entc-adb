#include "eccrypt.h"
#include "tools/eclog.h"
#include "types/ecstream.h"

#include <openssl/aes.h>
#include <openssl/evp.h>
#include <openssl/err.h>
#include <openssl/ssl.h>

#if defined _WIN32 || defined _WIN64

//=============================================================================

HCRYPTPROV ecencrypt_aes_acquireContext (unsigned int type, EcErr err)
{
  HCRYPTPROV handle;  
  
  /*
  if (!CryptAcquireContextW(&hProv, NULL, info, PROV_RSA_AES, CRYPT_VERIFYCONTEXT))
  {
    return NULL;
  }
  */
  
  return handle;
}

//=============================================================================

struct EcEncryptAES_s
{
  
  int64_t lenTotal;
  
};

//-----------------------------------------------------------------------------

void ecencrypt_aes_destroy (EcEncryptAES* pself)
{

}

//-----------------------------------------------------------------------------

EcEncryptAES ecencrypt_aes_initialize (const EcString secret, unsigned int type, unsigned int padding, EcErr err)
{
  
}

//-----------------------------------------------------------------------------

EcBuffer ecencrypt_aes_update (EcEncryptAES self, EcBuffer source, EcErr err)
{

}

//-----------------------------------------------------------------------------

EcBuffer ecencrypt_aes_finalize (EcEncryptAES self, EcErr err)
{

}

//=============================================================================

struct EcDecryptAES_s
{

	int dummy;

};

//-----------------------------------------------------------------------------

EcDecryptAES ecdecrypt_aes_initialize (const EcString secret, unsigned int type, unsigned int padding, EcErr err)
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

EcString eccrypt_aes_getkey (const EcString secret, int padding, const EVP_CIPHER* cypher, EcErr err)
{
  switch (padding)
  {
    case ENTC_KEY_PADDING_SHA256:
    {
      int keyLength = EVP_CIPHER_key_length (cypher);
      
      // length in 8 bit blocks
      if (keyLength == 32)   // 8 * 32 = 256
      {
        EcBuffer_s h;
        EcBuffer key;

        h.buffer = (unsigned char*)secret;
        h.size = ecstr_len(secret);
        
        // convert key into sha256 buffer, which has exactly the correct size (no padding needed)
        key = ecbuf_sha_256 (&h, err);
        if (key == NULL)
        {
          return NULL;
        }
        
        return ecbuf_str (&key);     
      }
      else
      {
        eclog_fmt (LL_ERROR, "ENTC", "eccrypt", "cypher has not supported key-length for padding (SHA256): %i", keyLength);
      }
      
      return NULL;
    }
    case ENTC_KEY_PADDING_ANSI_X923:
    {
      int size = ecstr_len (secret);

      // cypher options
      int keyLength = EVP_CIPHER_key_length (cypher);
      int blockSize = EVP_CIPHER_block_size (cypher);
      
      int diffSize = keyLength - size;
      
      // using the whole keylength for padding
      EcBuffer key = ecbuf_create (keyLength);

      eclog_fmt (LL_TRACE, "ENTC", "eccrypt", "padding (ANSI X.923) with key-length %i, block-size %i", keyLength, blockSize);
      
      // fill the buffer with they key
      memcpy (key->buffer, secret, size);
      
      // add the zeros (padding)
      memset (key->buffer + size, 0, diffSize);

      // add the last byte (padding)
      memset (key->buffer + keyLength - 1, diffSize, 1);
      
      // for debug
      {
        EcBuffer h = ecbuf_bin2hex(key);
        
        eclog_fmt (LL_TRACE, "ENTC", "eccrypt", ecbuf_const_str(h));
      }

      return ecbuf_str (&key);     
    }
  }
  
  return NULL;
}

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

//-----------------------------------------------------------------------------

const EVP_CIPHER* eccrypt_aes_getCipher (unsigned int type)
{
  switch (type)
  {
    case ENTC_AES_TYPE_CBC: return EVP_aes_256_cbc();
    case ENTC_AES_TYPE_CFB_1: return EVP_aes_256_cfb1();
    case ENTC_AES_TYPE_CFB_8: return EVP_aes_256_cfb8();
    case ENTC_AES_TYPE_CFB_128: return EVP_aes_256_cfb128();
  }
  
  return EVP_aes_256_cbc();
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

EcEncryptAES ecencrypt_aes_initialize (const EcString secret, unsigned int type, unsigned int padding, EcErr err)
{
  int res;  
  EcEncryptAES self = ecencrypt_aes_create ();
  
  // get the cypher
  const EVP_CIPHER* cypher = eccrypt_aes_getCipher (type);
  
  EcString key = eccrypt_aes_getkey (secret, padding, cypher, err);
  if (key == NULL)
  {
    ecencrypt_aes_destroy (&self);
    
    return NULL;
  }
    
  res = EVP_EncryptInit (&(self->ctx), cypher, (unsigned char*)key, NULL);
  
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

EcDecryptAES ecdecrypt_aes_initialize (const EcString secret, unsigned int type, unsigned int padding, EcErr err)
{
  int res;
  EcDecryptAES self = ecdecrypt_aes_create ();
  
  // get the cypher
  const EVP_CIPHER* cypher = eccrypt_aes_getCipher (type);
  
  EcString key = eccrypt_aes_getkey (secret, padding, cypher, err);
  if (key == NULL)
  {
    ecdecrypt_aes_destroy (&self);
    
    return NULL;
  }
  
  res = EVP_DecryptInit (&(self->ctx), cypher, (unsigned char*)key, NULL);
  
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

int ecencrypt_file (const EcString source, const EcString dest, const EcString secret, unsigned int type, EcErr err)
{
  EcEncryptAES aes;
  EcFileHandle fhIn;
  EcFileHandle fhOut;
  
  int res;
  
  aes = ecencrypt_aes_initialize (secret, type, 0, err);
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

int ecdecrypt_file (const EcString source, const EcString dest, const EcString secret, unsigned int type, EcErr err)
{
  EcDecryptAES aes;
  EcFileHandle fhIn;
  EcFileHandle fhOut;
  
  int res;
  
  aes = ecdecrypt_aes_initialize (secret, type, 0, err);
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
