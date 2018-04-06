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

typedef struct
{
  
  EcBuffer key;
  
  EcBuffer iv;

  EcBuffer salt;
  
} EcCryptKeys;

//----------------------------------------------------------------------------------------

EcCryptKeys* eccrypt_aes_getkey (const EcString secret, uint_t key_type, const EVP_CIPHER* cypher, EcBuffer source, EcErr err)
{
  switch (key_type)
  {
    case ENTC_KEY_PADDING_ZEROS:
    {
      EcCryptKeys* keys = ENTC_NEW (EcCryptKeys);
      
      int size = ecstr_len (secret);

      // cypher options
      int keyLength = EVP_CIPHER_key_length (cypher);
      
      // using the whole keylength for padding
      keys->key = ecbuf_create (keyLength);
      
      // add the zeros (padding)
      memset (keys->key->buffer, 0, keyLength);

      // fill the buffer with they key
      memcpy (keys->key->buffer, secret, size);
      
      // the rest is empty
      keys->iv = NULL;
      keys->salt = NULL;
      
      return keys;
    }
    case ENTC_KEY_PADDING_SHA256:
    {
      int keyLength = EVP_CIPHER_key_length (cypher);
      
      // length in 8 bit blocks
      if (keyLength == 32)   // 8 * 32 = 256
      {
        EcCryptKeys* keys = ENTC_NEW (EcCryptKeys);

        EcBuffer_s h;

        h.buffer = (unsigned char*)secret;
        h.size = ecstr_len(secret);
        
        // convert key into sha256 buffer, which has exactly the correct size (no padding needed)
        keys->key = ecbuf_sha_256 (&h, err);
        if (keys->key == NULL)
        {
          return NULL;
        }
        
        // the rest is empty
        keys->iv = NULL;
        keys->salt = NULL;
        
        return keys;
      }
      else
      {
        eclog_fmt (LL_ERROR, "ENTC", "eccrypt", "cypher has not supported key-length for padding (SHA256): %i", keyLength);
      }
      
      return NULL;
    }
    case ENTC_KEY_PADDING_ANSI_X923:
    {
      EcCryptKeys* keys = ENTC_NEW (EcCryptKeys);

      int size = ecstr_len (secret);

      // cypher options
      int keyLength = EVP_CIPHER_key_length (cypher);
      
      // using the whole keylength for padding
      keys->key = ecbuf_create (keyLength);

      //eclog_fmt (LL_TRACE, "ENTC", "eccrypt", "padding (ANSI X.923) with key-length %i, filling %i", keyLength, keyLength - size);
      
      // add the zeros (padding)
      memset (keys->key->buffer, 0, keyLength);

      // fill the buffer with they key
      memcpy (keys->key->buffer, secret, size);
      
      // add the last byte (padding)
      memset (keys->key->buffer + keyLength - 1, keyLength - size, 1);
      
      // the rest is empty
      keys->iv = NULL;
      keys->salt = NULL;
      
      return keys;
    }
    case ENTC_KEY_PADDING_PKCS7:
    {
      EcCryptKeys* keys = ENTC_NEW (EcCryptKeys);

      int size = ecstr_len (secret);

      // cypher options
      int keyLength = EVP_CIPHER_key_length (cypher);
      
      int diff = keyLength - size;
            
      // using the whole keylength for padding
      keys->key = ecbuf_create (keyLength);

      // add the padding
      memset (keys->key->buffer, diff, keyLength);

      // fill the buffer with they key
      memcpy (keys->key->buffer, secret, size);
      
      // the rest is empty
      keys->iv = NULL;
      keys->salt = NULL;
      
      return keys;
    }
    case ENTC_KEY_PASSPHRASE_DATA:
    {
      EcCryptKeys* keys = ENTC_NEW (EcCryptKeys);
      
      keys->key = ecbuf_create(32);
      keys->iv = ecbuf_create(16);
      
      EVP_BytesToKey(cypher, EVP_md5(), source->buffer + 8, (unsigned char*)secret, ecstr_len(secret), 1, keys->key->buffer, keys->iv->buffer);
     
      keys->key->buffer[265] = 0;
      keys->iv->buffer[128] = 0;
      
      keys->key = ecbuf_bin2hex(keys->key);
      keys->iv = ecbuf_bin2hex(keys->iv);
      
      ecstr_toUpper(keys->key->buffer);
      ecstr_toUpper(keys->iv->buffer);
      
      {
        /*
        EcBuffer h1 =
        EcBuffer h2 = ecbuf_bin2hex(keys->iv);
         */
        
        eclog_fmt (LL_TRACE, "ENTC", "eccrypt", "secret as passphrase key: %s", ecbuf_const_str(keys->key));
        eclog_fmt (LL_TRACE, "ENTC", "eccrypt", "secret as passphrase iv: %s", ecbuf_const_str(keys->iv));
        
        return keys;
      }
      
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

const EVP_CIPHER* eccrypt_aes_getCipher (uint_t type)
{
  switch (type)
  {
    case ENTC_AES_TYPE_CBC:
    {
      return EVP_aes_256_cbc();
    }
    case ENTC_AES_TYPE_CFB:
    {
      return EVP_aes_256_cfb();
    }
    case ENTC_AES_TYPE_CFB_1:
    {
      return EVP_aes_256_cfb1();
    }
    case ENTC_AES_TYPE_CFB_8:
    {
      return EVP_aes_256_cfb8();
    }
    case ENTC_AES_TYPE_CFB_128:
    {
      return EVP_aes_256_cfb128();
    }
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
  
  uint_t cypher_type;
  
  uint_t key_type;
  
  // this are our secrets
  
  EcString secret;
  
  EcCryptKeys* keys;
  
};

//-----------------------------------------------------------------------------

EcEncryptAES ecencrypt_aes_create (const EcString secret, uint_t cypher_type, uint_t key_type)
{
  EcEncryptAES self = ENTC_NEW(struct EcEncryptAES_s);
  
  EVP_CIPHER_CTX_init (&(self->ctx));
  
  self->blocksize = 0;
  self->lenTotal = 0;
  
  self->buf = NULL;
  
  self->cypher_type = cypher_type;
  self->key_type = key_type;
  
  self->secret = ecstr_copy (secret);
  self->keys = NULL;
  
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

int ecencrypt_aes_initialize (EcEncryptAES self, EcBuffer source, EcErr err)
{
  int res;  
  
  // get the cypher
  const EVP_CIPHER* cypher = eccrypt_aes_getCipher (self->cypher_type);
  
  self->keys = eccrypt_aes_getkey (self->secret, self->key_type, cypher, source, err);
  if (self->keys == NULL)
  {
    return err->code;
  }
    
  res = EVP_EncryptInit (&(self->ctx), cypher, (unsigned char*)self->keys->key, (unsigned char*)self->keys->iv);
  
  if (res == 0)
  {
    eccrypt_aes_handleError (&(self->ctx), err);
    
    return err->code;
  }
  
  // check for the blocksize
  self->blocksize = EVP_CIPHER_CTX_block_size(&(self->ctx));

  return ENTC_ERR_NONE;
}

//-----------------------------------------------------------------------------

EcBuffer ecencrypt_aes_update (EcEncryptAES self, EcBuffer source, EcErr err)
{
  if (self->keys == NULL)
  {
    int res = ecencrypt_aes_initialize (self, source, err);
    if (res)
    {
      return NULL;
    }
  }
  
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

  uint_t cypher_type;
  
  uint_t key_type;
  
  // this are our secrets
  
  EcString secret;
  
  EcCryptKeys* keys;

};

//-----------------------------------------------------------------------------

EcDecryptAES ecdecrypt_aes_create (const EcString secret, uint_t cypher_type, uint_t key_type)
{
  EcDecryptAES self = ENTC_NEW(struct EcDecryptAES_s);
  
  EVP_CIPHER_CTX_init (&(self->ctx));

  self->blocksize = 0;
  self->lenTotal = 0;
  
  self->buf = NULL;
  
  self->cypher_type = cypher_type;
  self->key_type = key_type;
  
  self->secret = ecstr_copy (secret);
  self->keys = NULL;
  
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

int ecdecrypt_aes_initialize (EcDecryptAES self, EcBuffer source, EcErr err)
{
  int res;
  
  // get the cypher
  const EVP_CIPHER* cypher = eccrypt_aes_getCipher (self->cypher_type);
  
  self->keys = eccrypt_aes_getkey (self->secret, self->key_type, cypher, source, err);
  if (self->keys == NULL)
  {
    return err->code;
  }
  
  res = EVP_DecryptInit (&(self->ctx), cypher, (unsigned char*)self->keys->key, (unsigned char*)self->keys->iv);
  if (res == 0)
  {
    eccrypt_aes_handleError (&(self->ctx), err);
    
    return err->code;
  }
  
  // check for the blocksize
  self->blocksize = EVP_CIPHER_CTX_block_size(&(self->ctx));
  
  return ENTC_ERR_NONE;
}

//-----------------------------------------------------------------------------

EcBuffer ecdecrypt_aes_update (EcDecryptAES self, EcBuffer source, EcErr err)
{
  if (self->keys == NULL)
  {
    int res = ecdecrypt_aes_initialize (self, source, err);
    if (res)
    {
      return NULL;
    }
  }
  
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
  
  aes = ecencrypt_aes_create (secret, type, 0);
  
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
  
  aes = ecdecrypt_aes_create (secret, type, 0);
  
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
