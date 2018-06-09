#include "eccrypt.h"

#include "tools/eclog.h"
#include "tools/eccode.h"

#include "types/ecstream.h"

#if defined _WIN32 || defined _WIN64

#include <windows.h>
#include <wincrypt.h>
#pragma comment (lib, "Crypt32.lib")

//=============================================================================

HCRYPTPROV ecencrypt_aes_acquireContext (DWORD provType, EcErr err)
{
  HCRYPTPROV provHandle = (HCRYPTPROV)NULL; 
  
  if (!CryptAcquireContext (&provHandle, NULL, NULL, provType, 0))
  {
    DWORD errCode = GetLastError ();

    if (errCode == NTE_BAD_KEYSET)
    {
      if (!CryptAcquireContext (&provHandle, NULL, NULL, provType, CRYPT_NEWKEYSET))
      {
        ecerr_lastErrorOS (err, ENTC_LVL_ERROR);
        return NULL;
      }
    }
  }
  
  return provHandle;
}

//=============================================================================

struct EcEncryptAES_s
{

	int dummy;

};

//-----------------------------------------------------------------------------

EcEncryptAES ecencrypt_aes_create (uint_t cypher_type, uint_t padding_type, const EcString secret, uint_t key_type)
{

}

//-----------------------------------------------------------------------------

void ecencrypt_aes_destroy (EcEncryptAES* pself)
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

EcDecryptAES ecdecrypt_aes_create (const EcString secret, uint_t cypher_type, uint_t key_type)
{

}

//-----------------------------------------------------------------------------

void ecdecrypt_aes_destroy (EcDecryptAES* pself)
{

}

//-----------------------------------------------------------------------------

EcBuffer ecdecrypt_aes_update (EcDecryptAES self, EcBuffer source, EcErr err)
{

}

//-----------------------------------------------------------------------------

EcBuffer ecdecrypt_aes_finalize (EcDecryptAES self, EcErr err)
{

}

//-----------------------------------------------------------------------------

#elif defined __BSD_OS || defined __LINUX_OS

#include <openssl/aes.h>
#include <openssl/evp.h>
#include <openssl/err.h>
#include <openssl/ssl.h>

//----------------------------------------------------------------------------------------

typedef struct
{
  
  EcBuffer key;
  
  EcBuffer iv;
  
} EcCryptKeys;

//-----------------------------------------------------------------------------

EcCryptKeys* eccrypt_padding_sha256 (const EcString secret, const EVP_CIPHER* cypher, EcErr err)
{
  int keyLength = EVP_CIPHER_key_length (cypher);
  
  //eclog_fmt (LL_TRACE, "ENTC", "eccrypt", "padding (SHA256) with key-length %i", keyLength);
  
  // length in 8 bit blocks
  if (keyLength == 32)   // 8 * 32 = 256
  {
    EcCryptKeys* keys = ENTC_NEW (EcCryptKeys);

    EcBuffer_s h;

    //eclog_fmt (LL_TRACE, "ENTC", "eccrypt", "padding (SHA256) with secret %s", secret);
    
    h.buffer = (unsigned char*)secret;
    h.size = ecstr_len(secret);
    
    // convert key into sha256 buffer, which has exactly the correct size (no padding needed)
    keys->key = echash_sha256 (&h, err);
    if (keys->key == NULL)
    {
      ENTC_DEL (&keys, EcCryptKeys);
      return NULL;
    }
    
    //eclog_fmt (LL_TRACE, "ENTC", "eccrypt", "padding (SHA256) with buf-length %i | %s", keys->key->size, keys->key->buffer);
    
    // the rest is empty
    keys->iv = NULL;
    
    return keys;
  }

  eclog_fmt (LL_ERROR, "ENTC", "eccrypt", "cypher has not supported key-length for padding (SHA256): %i", keyLength);

  return NULL;
}

//-----------------------------------------------------------------------------

EcCryptKeys* eccrypt_padding_zero (const EcString secret, const EVP_CIPHER* cypher)
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
  
  return keys;
}
    
//-----------------------------------------------------------------------------

void eccrypt_padding_ansiX923_pad (EcBuffer buffer, int64_t offset)
{
  int64_t diff = buffer->size - offset;  // calculate the difference
  
  // add the zeros (padding)
  memset (buffer->buffer + offset, 0, diff);

  // add the last byte (padding)
  memset (buffer->buffer + buffer->size - 1, diff, 1);
}

//-----------------------------------------------------------------------------

EcCryptKeys* eccrypt_padding_ansiX923 (const EcString secret, const EVP_CIPHER* cypher)
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
  
  return keys;
}

//-----------------------------------------------------------------------------

EcCryptKeys* eccrypt_padding_pkcs7 (const EcString secret, const EVP_CIPHER* cypher)
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
  
  return keys;  
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

//-----------------------------------------------------------------------------

unsigned char* eccrypt_keybuffer (EcBuffer buf)
{
  if (buf)
  {
    return buf->buffer;
  }

  return NULL;
}

//=============================================================================

struct EcEncryptAES_s
{
  
  EVP_CIPHER_CTX ctx;
  
  EcBuffer buf;
  
  int blocksize;
  
  int64_t lenTotal;
  
  uint_t cypher_type;
  
  uint_t padding_type;
  
  uint_t key_type;
  
  // this are our secrets
  
  EcString secret;
  
  EcCryptKeys* keys;
  
  int bufoffset;
  
};

//-----------------------------------------------------------------------------

EcEncryptAES ecencrypt_aes_create (uint_t cypher_type, uint_t padding_type, const EcString secret, uint_t key_type)
{
  EcEncryptAES self = ENTC_NEW(struct EcEncryptAES_s);
  
  EVP_CIPHER_CTX_init (&(self->ctx));
  
  self->blocksize = 0;
  self->lenTotal = 0;
  
  self->buf = NULL;

  // cipher settings
  self->cypher_type = cypher_type;
  self->padding_type = padding_type;
  
  // key settings
  self->secret = ecstr_copy (secret);
  self->key_type = key_type;
  
  self->keys = NULL;
  self->bufoffset = 0;
  
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
  
  if (self->keys)
  {
      if (self->keys->key)
      {
        ecbuf_destroy (&(self->keys->key));
      }
      
      if (self->keys->iv)
      {
        ecbuf_destroy (&(self->keys->iv));
      }
      
      ENTC_DEL (&(self->keys), EcCryptKeys);
  }
  
  ecstr_delete (&(self->secret));
  
  ENTC_DEL(pself, struct EcEncryptAES_s);
}

//-----------------------------------------------------------------------------

int ecencrypt_aes_initialize (EcEncryptAES self, EcBuffer source, EcErr err)
{
  int res;  
  uint_t offset = 0;
  
  // get the cypher
  const EVP_CIPHER* cypher = eccrypt_aes_getCipher (self->cypher_type);
    
  switch (self->key_type)
  {
    case ENTC_KEY_SHA256:
    {
      self->keys = eccrypt_padding_sha256 (self->secret, cypher, err);
      break;
    }
    case ENTC_PADDING_ZEROS:
    {
      self->keys = eccrypt_padding_zero (self->secret, cypher);
      break;
    }
    case ENTC_PADDING_ANSI_X923:
    {
      self->keys = eccrypt_padding_ansiX923 (self->secret, cypher);
      break;
    }
    case ENTC_PADDING_PKCS7:
    {
      self->keys = eccrypt_padding_pkcs7 (self->secret, cypher);
      break;
    }
    case ENTC_KEY_PASSPHRASE_MD5:
    {
      int rounds = 1;
            
      self->keys = ENTC_NEW (EcCryptKeys);
      
      self->keys->key = ecbuf_create (EVP_MAX_KEY_LENGTH);
      self->keys->iv = ecbuf_create (EVP_MAX_IV_LENGTH);
      
      self->buf = ecbuf_create (16);
      
      ecbuf_random(self->buf, 16);
      memcpy (self->buf->buffer, "Salted__", 8);
      
      res = EVP_BytesToKey (cypher, EVP_md5(), self->buf->buffer + 8, (unsigned char*)self->secret, ecstr_len(self->secret), rounds, self->keys->key->buffer, self->keys->iv->buffer);

      self->keys->key->size = res;

      // for debug
      /*
      {
          EcBuffer a0 = ecbuf_create_buffer_cp(self->buf->buffer + 8, 8);
          
          EcBuffer h0 = ecbuf_bin2hex (a0);
          EcBuffer h1 = ecbuf_bin2hex (self->keys->key);
          EcBuffer h2 = ecbuf_bin2hex (self->keys->iv);
          
          eclog_fmt (LL_TRACE, "ENTC", "eccrypt", "SLT: %s", h0->buffer);
          eclog_fmt (LL_TRACE, "ENTC", "eccrypt", "KEY: %s", h1->buffer);
          eclog_fmt (LL_TRACE, "ENTC", "eccrypt", " IV: %s", h2->buffer);
          
          ecbuf_destroy(&h1);
          ecbuf_destroy(&h2);
          ecbuf_destroy(&h0);
          ecbuf_destroy(&a0);
      }
      */
      
      self->bufoffset = 16;
      
      break;
    }
  }
  
  if (self->keys == NULL)
  {
    return ecerr_set (err, ENTC_LVL_ERROR, ENTC_ERR_WRONG_STATE, "decoding of secret failed");    
  }
    
  res = EVP_EncryptInit_ex (&(self->ctx), cypher, NULL, eccrypt_keybuffer (self->keys->key), eccrypt_keybuffer (self->keys->iv));
  
  if (res == 0)
  {
    eccrypt_aes_handleError (&(self->ctx), err);
    
    return err->code;
  }
  
  // check for the blocksize
  self->blocksize = EVP_CIPHER_CTX_block_size(&(self->ctx));
  
  if (self->padding_type)
  {
    // disable automatic padding 
    EVP_CIPHER_CTX_set_padding (&(self->ctx), 0);
  }
  else
  {
    EVP_CIPHER_CTX_set_padding (&(self->ctx), 1);
  }
  
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
    ecbuf_resize (self->buf, self->bufoffset + source->size + self->blocksize);
  }
  else
  {
    self->buf = ecbuf_create (self->bufoffset + source->size + self->blocksize);
  }
  
  int lenLast;
  
  if (EVP_EncryptUpdate(&(self->ctx), self->buf->buffer + self->bufoffset, &lenLast, source->buffer, source->size) == 0)
  {
    eccrypt_aes_handleError (&(self->ctx), err);
    return NULL;
  }
  
  lenLast += self->bufoffset;
  
  self->buf->size = lenLast;
  self->lenTotal += lenLast;
  
  self->bufoffset = 0;

  return self->buf;
}

//-----------------------------------------------------------------------------

void ecencrypt_aes_reserveBuffer (EcEncryptAES self, int64_t offset)
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
}

//-----------------------------------------------------------------------------

EcBuffer ecencrypt_aes_finalize (EcEncryptAES self, EcErr err)
{
  switch (self->padding_type)
  {
    default:
    {
      ecencrypt_aes_reserveBuffer (self, 0);
      
      self->bufoffset = 0;
      
      break;
    }
    case ENTC_PADDING_ANSI_X923:   // force padding
    {
      EcBuffer padding;
      
      // we need to encrypt the padding
      // calculate how much we need to pad
      uint64_t padlen = ((self->lenTotal / 8) + 1) * 8 - self->lenTotal;
      
      padding = ecbuf_create (padlen);
      
      printf ("PAD: padding %u\n", padlen);
      
      eccrypt_padding_ansiX923_pad (padding, 0);
      
      ecencrypt_aes_reserveBuffer (self, padlen);

      {
        int lenLast;
        if (EVP_EncryptUpdate(&(self->ctx), self->buf->buffer, &lenLast, padding->buffer, padding->size) == 0)
        {
          eccrypt_aes_handleError (&(self->ctx), err);
          return NULL;
        }
        
        self->bufoffset = lenLast;
        self->lenTotal += lenLast;
      }
        
      break;
    }
  }
  
  int lenLast;

  if (EVP_EncryptFinal_ex(&(self->ctx), self->buf->buffer + self->bufoffset, &lenLast) == 0)
  {
    eccrypt_aes_handleError (&(self->ctx), err);
    return NULL;
  }

  lenLast += self->bufoffset;
  
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
  
  ecstr_delete(&(self->secret));
  
  if (self->keys)
  {
      if (self->keys->key)
      {
        ecbuf_destroy (&(self->keys->key));
      }
      
      if (self->keys->iv)
      {
        ecbuf_destroy (&(self->keys->iv));
      }
      
      ENTC_DEL(&(self->keys), EcCryptKeys);
  }

  ENTC_DEL(pself, struct EcDecryptAES_s);
}

//-----------------------------------------------------------------------------

int ecdecrypt_aes_initialize (EcDecryptAES self, EcBuffer source, uint_t* bufoffset, EcErr err)
{
  int res;
  
  // get the cypher
  const EVP_CIPHER* cypher = eccrypt_aes_getCipher (self->cypher_type);
  
  switch (self->key_type)
  {
    case ENTC_KEY_SHA256:
    {
      self->keys = eccrypt_padding_sha256 (self->secret, cypher, err);
      break;
    }
    case ENTC_PADDING_ZEROS:
    {
      self->keys = eccrypt_padding_zero (self->secret, cypher);
      break;
    }
    case ENTC_PADDING_ANSI_X923:
    {
      self->keys = eccrypt_padding_ansiX923 (self->secret, cypher);
      break;
    }
    case ENTC_PADDING_PKCS7:
    {
      self->keys = eccrypt_padding_pkcs7 (self->secret, cypher);
      break;
    }
    case ENTC_KEY_PASSPHRASE_MD5:
    {
      // do some pre-checks
      if (source->size > 16)
      {
        EcCryptKeys* keys = ENTC_NEW (EcCryptKeys);
        
        // cypher options
        int keyLength = EVP_CIPHER_key_length (cypher);
        
        //eclog_fmt (LL_TRACE, "ENTC", "eccrypt", "passphrase with key-length %i", keyLength);
        
        keys->key = ecbuf_create (keyLength);
        keys->iv = ecbuf_create (16);
        
        *bufoffset = 16;
        
        EVP_BytesToKey (cypher, EVP_md5(), source->buffer + 8, (unsigned char*)self->secret, ecstr_len(self->secret), 1, keys->key->buffer, keys->iv->buffer);
        
        self->keys = keys;
        
        // for debug
        /*
        {
          EcBuffer a0 = ecbuf_create_buffer_cp(source->buffer + 8, 8);
          
          EcBuffer h0 = ecbuf_bin2hex (a0);
          EcBuffer h1 = ecbuf_bin2hex (self->keys->key);
          EcBuffer h2 = ecbuf_bin2hex (self->keys->iv);
          
          eclog_fmt (LL_TRACE, "ENTC", "eccrypt", "SLT: %s", h0->buffer);
          eclog_fmt (LL_TRACE, "ENTC", "eccrypt", "KEY: %s", h1->buffer);
          eclog_fmt (LL_TRACE, "ENTC", "eccrypt", " IV: %s", h2->buffer);
          
          ecbuf_destroy(&h1);
          ecbuf_destroy(&h2);
          ecbuf_destroy(&h0);
          ecbuf_destroy(&a0);
        }
        */
      }
      else
      {
        eclog_fmt (LL_ERROR, "ENTC", "eccrypt", "source buffer has less than 16 bytes");

        return ecerr_set (err, ENTC_LVL_ERROR, ENTC_ERR_WRONG_STATE, "source buffer has less than 16 bytes");
      }
      
      break;
    }
  }
  
  if (self->keys == NULL)
  {
    return ecerr_set (err, ENTC_LVL_ERROR, ENTC_ERR_WRONG_STATE, "decoding of secret failed");    
  }
      
  res = EVP_DecryptInit_ex (&(self->ctx), cypher, NULL, eccrypt_keybuffer (self->keys->key), eccrypt_keybuffer (self->keys->iv));
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
  uint_t bufoffset = 0;
  
  if (self->keys == NULL)
  {
    int res = ecdecrypt_aes_initialize (self, source, &bufoffset, err);
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
  
  if (EVP_DecryptUpdate (&(self->ctx), self->buf->buffer, &lenLast, source->buffer + bufoffset, source->size - bufoffset) == 0)
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

int ecencrypt_file_copy (EcEncryptAES aes, EcFileHandle fhIn, EcFileHandle fhOut, EcErr err)
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
    
    ben = ecencrypt_aes_update (aes, &h, err);
    if (ben == NULL)
    {
      return err->code;
    }
    
    bytesWritten = ecfh_writeBuffer (fhOut, ben, ben->size);
    if (bytesWritten < 0)
    {
      eclog_fmt (LL_ERROR, "ENTC", "enc file", "can't write bytes");
      
      return ecerr_set (err, ENTC_LVL_ERROR, ENTC_ERR_PROCESS_FAILED, "can't write bytes");
    }
  }
  
  ben = ecencrypt_aes_finalize (aes, err);
  if (ben == NULL)
  {
    return err->code;
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
  
  aes = ecencrypt_aes_create (type, 0, secret, 0);
  
  fhIn = ecfh_open (source, O_RDONLY);
  if (fhIn == NULL)
  {
    ecencrypt_aes_destroy (&aes);
    
    eclog_fmt (LL_WARN, "ENTC", "enc file", "can't open original file '%s'", source);
    
    return ecerr_set (err, ENTC_LVL_ERROR, ENTC_ERR_PROCESS_FAILED, "can't open original file");
  }
  
  fhOut = ecfh_open (dest, O_CREAT | O_TRUNC | O_WRONLY);
  if (fhOut == NULL)
  {
    ecfh_close (&fhIn);
    
    ecencrypt_aes_destroy (&aes);
    
    eclog_fmt (LL_WARN, "ENTC", "enc file", "can't open dest file '%s'", dest);
    
    return ecerr_set (err, ENTC_LVL_ERROR, ENTC_ERR_PROCESS_FAILED, "can't open dest file");
  }
  
  eclog_fmt (LL_TRACE, "ENTC", "end file", "encrypt file");
  
  res = ecencrypt_file_copy (aes, fhIn, fhOut, err);
  
  ecfh_close (&fhIn);
  ecfh_close (&fhOut);
  
  ecencrypt_aes_destroy (&aes);
  
  return res;
}

//-------------------------------------------------------------------------------------------------------

int ecdecrypt_file_copy (EcDecryptAES aes, EcFileHandle fhIn, EcFileHandle fhOut, EcErr err)
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
    
    ben = ecdecrypt_aes_update (aes, &h, err);
    if (ben == NULL)
    {
      return err->code;
    }
    
    bytesWritten = ecfh_writeBuffer (fhOut, ben, ben->size);
    if (bytesWritten < 0)
    {
      eclog_fmt (LL_ERROR, "ENTC", "dec file", "can't write bytes");
      
      return ecerr_set (err, ENTC_LVL_ERROR, ENTC_ERR_PROCESS_FAILED, "can't write bytes");
    }
  }
  
  ben = ecdecrypt_aes_finalize (aes, err);
  if (ben == NULL)
  {
    return err->code;
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
    
    return ecerr_set (err, ENTC_LVL_ERROR, ENTC_ERR_PROCESS_FAILED, "can't open original file");
  }
  
  fhOut = ecfh_open (dest, O_CREAT | O_TRUNC | O_WRONLY);
  if (fhOut == NULL)
  {
    ecfh_close (&fhIn);
    
    ecdecrypt_aes_destroy (&aes);
    
    eclog_fmt (LL_WARN, "ENTC", "dec file", "can't open dest file '%s'", dest);
    
    return ecerr_set (err, ENTC_LVL_ERROR, ENTC_ERR_PROCESS_FAILED, "can't open dest file");
  }
  
  eclog_fmt (LL_TRACE, "ENTC", "dec file", "decrypt file");
  
  res = ecdecrypt_file_copy (aes, fhIn, fhOut, err);
  
  ecfh_close (&fhIn);
  ecfh_close (&fhOut);
  
  ecdecrypt_aes_destroy (&aes);
  
  return res;
}

//-----------------------------------------------------------------------------
