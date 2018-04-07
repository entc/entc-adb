#include "echash.h"

//=============================================================================

#include <openssl/sha.h>

EcBuffer echash_sha256 (EcBuffer source, EcErr err)
{
  EcBuffer bout = ecbuf_create (SHA256_DIGEST_LENGTH);
  
  SHA256_CTX sha256;
  
  SHA256_Init (&sha256);
  
  SHA256_Update (&sha256, source->buffer, source->size);
  
  SHA256_Final(bout->buffer, &sha256);
  
  return bout;
}

//-----------------------------------------------------------------------------
