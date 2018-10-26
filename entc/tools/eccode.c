#include "eccode.h"

#include "system/macros.h"

//=============================================================================

#if defined _WIN32 || defined _WIN64

#include <windows.h>
#include <wincrypt.h>
#pragma comment (lib, "Crypt32.lib")

//=============================================================================

HCRYPTPROV echash_prov_handle (DWORD provType, EcErr err)
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

//-----------------------------------------------------------------------------

int echash_hash_retrieve (EcBuffer source, EcBuffer dest, HCRYPTHASH hashHandle, EcErr err)
{
  if (CryptHashData (hashHandle, source->buffer, source->size, 0) == 0)
  {
    return ecerr_lastErrorOS (err, ENTC_LVL_ERROR);
  }

  if (CryptGetHashParam (hashHandle, HP_HASHVAL, dest->buffer, &(dest->size), 0) == 0)
  {
    return ecerr_lastErrorOS (err, ENTC_LVL_ERROR);
  } 
  
  return ENTC_ERR_NONE;
}

//-----------------------------------------------------------------------------

EcBuffer echash_sha_1 (EcBuffer source, EcErr err)
{
  int res;
  EcBuffer ret = NULL;

  HCRYPTPROV provHandle = (HCRYPTPROV)NULL;
  HCRYPTHASH hashHandle;
  
  provHandle = echash_prov_handle (PROV_RSA_AES, err);
  if (provHandle == NULL)
  {
    return NULL;
  }
  
  if (!CryptCreateHash (provHandle, CALG_SHA1, 0, 0, &hashHandle))
  {
    ecerr_lastErrorOS (err, ENTC_LVL_ERROR);
    CryptReleaseContext (provHandle, 0);

    return NULL;
  }
  
  // create the buffer for the hash
  ret = ecbuf_create (16);

  // retrieve the hash from windows  
  res = echash_hash_retrieve (source, ret, hashHandle, err);
  if (res)
  {
    // some error happened -> destroy the buffer
    ecbuf_destroy (&ret);
  }
  
  CryptDestroyHash (hashHandle);
  CryptReleaseContext (provHandle, 0);
  
  return ret;   
}

//-----------------------------------------------------------------------------

EcBuffer echash_sha256 (EcBuffer source, EcErr err)
{
  int res;
  EcBuffer ret = NULL;

  HCRYPTPROV provHandle = (HCRYPTPROV)NULL;
  HCRYPTHASH hashHandle;
  
  provHandle = echash_prov_handle (PROV_RSA_AES, err);
  if (provHandle == NULL)
  {
    return NULL;
  }
    
  if (!CryptCreateHash (provHandle, CALG_SHA_256, 0, 0, &hashHandle))
  {
    ecerr_lastErrorOS (err, ENTC_LVL_ERROR);
    CryptReleaseContext (provHandle, 0);

    return NULL;
  }
  
  // create the buffer for the hash
  ret = ecbuf_create (32);

  // retrieve the hash from windows  
  res = echash_hash_retrieve (source, ret, hashHandle, err);
  if (res)
  {
    // some error happened -> destroy the buffer
    ecbuf_destroy (&ret);
  }
  
  CryptDestroyHash (hashHandle);
  CryptReleaseContext (provHandle, 0);
  
  return ret;
}

//-----------------------------------------------------------------------------

EcBuffer echash_md5 (EcBuffer source, EcErr err)
{
  int res;
  EcBuffer ret = NULL;

  HCRYPTPROV provHandle = (HCRYPTPROV)NULL;
  HCRYPTHASH hashHandle;
  
  provHandle = echash_prov_handle (PROV_RSA_FULL, err);
  if (provHandle == NULL)
  {
    return NULL;
  }
  
  if (!CryptCreateHash (provHandle, CALG_MD5, 0, 0, &hashHandle))
  {
    ecerr_lastErrorOS (err, ENTC_LVL_ERROR);
    CryptReleaseContext (provHandle, 0);

    return NULL;
  }
  
  // create the buffer for the hash
  ret = ecbuf_create (16);

  // retrieve the hash from windows  
  res = echash_hash_retrieve (source, ret, hashHandle, err);
  if (res)
  {
    // some error happened -> destroy the buffer
    ecbuf_destroy (&ret);
  }
  
  CryptDestroyHash (hashHandle);
  CryptReleaseContext (provHandle, 0);
  
  return ret;  
}

//-----------------------------------------------------------------------------

EcBuffer eccode_base64_encode (EcBuffer source)
{
  EcBuffer ret = ENTC_NEW (EcBuffer_s);

  // calculate the size of the buffer
  CryptBinaryToString (source->buffer, source->size, CRYPT_STRING_BASE64 | CRYPT_STRING_NOCRLF, NULL, &(ret->size));

  // allocate the buffer
  ret->buffer = (unsigned char*)ENTC_MALLOC (ret->size + 1);

  // encrypt
  CryptBinaryToString (source->buffer, source->size, CRYPT_STRING_BASE64 | CRYPT_STRING_NOCRLF, ret->buffer, &(ret->size));
  ret->buffer [ret->size] = 0; 

  return ret;  
}

//-----------------------------------------------------------------------------

EcBuffer eccode_base64_decode (EcBuffer source)
{
  EcBuffer ret = ENTC_NEW (EcBuffer_s);

  // calculate the size of the buffer
  CryptStringToBinary (source->buffer, 0, CRYPT_STRING_BASE64, NULL, &(ret->size), NULL, NULL); 

  // allocate the buffer
  ret->buffer = (unsigned char*)ENTC_MALLOC (ret->size + 1);

  // decrypt
  CryptStringToBinary (source->buffer, 0, CRYPT_STRING_BASE64, ret->buffer, &(ret->size), NULL, NULL);
  ret->buffer [ret->size] = 0; 

  return ret;  
}

//=============================================================================

struct EcBase64Encode_s
{
  
  int dyummy;
  
};

//-----------------------------------------------------------------------------

EcBase64Encode eccode_base64_encode_create (void)
{
  EcBase64Encode self = ENTC_NEW(struct EcBase64Encode_s);
  
 return self;
}

//-----------------------------------------------------------------------------

void eccode_base64_encode_destroy (EcBase64Encode* pself)
{
  ENTC_DEL (pself, struct EcBase64Encode_s);
}

//-----------------------------------------------------------------------------

uint_t eccode_base64_encode_update (EcBase64Encode self, EcBuffer dest, EcBuffer source, EcErr err)
{
  
}

//-----------------------------------------------------------------------------

uint_t eccode_base64_encode_finalize (EcBase64Encode self, EcBuffer dest, EcErr err)
{
  
}

//-----------------------------------------------------------------------------

uint_t eccode_base64_encode_sourceSize (uint_t max)
{
  return (max / 4 * 2.7) - 64;
}

//=============================================================================

#else

//=============================================================================

#include <openssl/sha.h>
#include <openssl/md5.h>
#include <openssl/evp.h>

EcBuffer echash_sha_1 (EcBuffer source, EcErr err)
{
  EcBuffer bout = ecbuf_create (SHA_DIGEST_LENGTH);
  
  SHA_CTX sha1;
  
  SHA1_Init (&sha1);
  
  SHA1_Init (&sha1);
  
  SHA1_Update (&sha1, source->buffer, source->size);
  
  SHA1_Final (bout->buffer, &sha1);
  
  return bout;
}

//-----------------------------------------------------------------------------

EcBuffer echash_sha256 (EcBuffer source, EcErr err)
{
  EcBuffer bout = ecbuf_create (SHA256_DIGEST_LENGTH);
  
  SHA256_CTX sha256;
  
  if (SHA256_Init (&sha256) == 0)
  {
    ecbuf_destroy (&bout);
    
    return NULL;    
  }
  
  if (SHA256_Update (&sha256, source->buffer, source->size) == 0)
  {
    ecbuf_destroy (&bout);
    
    return NULL;    
  }
  
  if (SHA256_Final(bout->buffer, &sha256) == 0)
  {
    ecbuf_destroy (&bout);
    
    return NULL;    
  }
  
  return bout;
}

//-----------------------------------------------------------------------------

EcBuffer echash_md5 (EcBuffer source, EcErr err)
{
  EcBuffer bout = ecbuf_create (MD5_DIGEST_LENGTH);
  
  MD5_CTX md5ctx;
  
  if (MD5_Init (&md5ctx) == 0)
  {
    ecbuf_destroy (&bout);
    
    return NULL;
  }
  
  if (MD5_Update (&md5ctx, source->buffer, source->size) == 0)
  {
    ecbuf_destroy (&bout);
    
    return NULL;
  }
  
  if (MD5_Final (bout->buffer, &md5ctx) == 0)
  {
    ecbuf_destroy (&bout);
    
    return NULL;
  }
  
  return bout;
}

//-----------------------------------------------------------------------------

EcBuffer eccode_base64_encode (EcBuffer source)
{
  EcBuffer ret = ecbuf_create (((source->size + 2) / 3 * 4) + 1);
  
  // openssl function
  int decodedSize = EVP_EncodeBlock (ret->buffer, source->buffer, source->size);
  
  // everything worked fine
  if ((decodedSize > 0) && (decodedSize < ret->size))
  {
    ret->size = decodedSize;
    return ret;
  }
  
  ecbuf_destroy (&ret);
  return NULL;
}

//-----------------------------------------------------------------------------

EcBuffer eccode_base64_decode (EcBuffer source)
{
  EcBuffer ret = ecbuf_create (((source->size + 3) / 4 * 3) + 1);
  
  // openssl function
  int decodedSize = EVP_DecodeBlock (ret->buffer, source->buffer, source->size);

  // everything worked fine
  if ((decodedSize > 0) && (decodedSize < ret->size))
  {
    // trim the last bytes which are 0
    while ((ret->buffer[decodedSize - 1] == '\0') && (decodedSize > 0))
    {
      decodedSize--;
    }
    
    ret->size = decodedSize;
    return ret;
  }

  ecbuf_destroy (&ret);
  return NULL;
}

//=============================================================================

struct EcBase64Encode_s
{
  
  EVP_ENCODE_CTX ctx;
  
};

//-----------------------------------------------------------------------------

EcBase64Encode eccode_base64_encode_create (void)
{
  EcBase64Encode self = ENTC_NEW(struct EcBase64Encode_s);
  
  EVP_EncodeInit (&(self->ctx));
  
  return self;
}

//-----------------------------------------------------------------------------

void eccode_base64_encode_destroy (EcBase64Encode* pself)
{
  ENTC_DEL (pself, struct EcBase64Encode_s);
}

//-----------------------------------------------------------------------------

uint_t eccode_base64_encode_update (EcBase64Encode self, EcBuffer dest, EcBuffer source, EcErr err)
{
  int len;
  
  EVP_EncodeUpdate (&(self->ctx), dest->buffer, &len, source->buffer, source->size);
  
  return len;
}

//-----------------------------------------------------------------------------

uint_t eccode_base64_encode_finalize (EcBase64Encode self, EcBuffer dest, EcErr err)
{
  int len;
  
  EVP_EncodeFinal (&(self->ctx), dest->buffer, &len);
  
  return len;
}

//-----------------------------------------------------------------------------

uint_t eccode_base64_encode_sourceSize (uint_t max)
{
  return (max / 4 * 2.7) - 64;
}

//-----------------------------------------------------------------------------

uint_t eccode_base64_encode_size (uint_t size)
{
  return ((size + 2) / 3 * 4) + 1 + 64;
}

//-----------------------------------------------------------------------------

#endif
