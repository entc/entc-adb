/*
 * Copyright (c) 2010-2013 "Alexander Kalkhof" [email:entc@kalkhof.org]
 *
 * This file is part of the extension n' tools (entc-base) framework for C.
 *
 * entc-base is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * entc-base is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with entc-base.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "ecbuffer.h"
#include "ecstream.h"

#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "md5.h"

#ifdef _WIN32
#include <windows.h>
#endif

//----------------------------------------------------------------------------------------

EcBuffer ecbuf_create (uint_t size)
{
  return ecbuf_create_filled (size, 0x00);  
}

//----------------------------------------------------------------------------------------

EcBuffer ecbuf_create_filled (uint_t size, char fillupwith)
{
  EcBuffer self = ENTC_NEW (EcBuffer_s);
  
  self->buffer =  (unsigned char*)ENTC_MALLOC( (1 + size) * sizeof(unsigned char) );
  self->size = size;
  
  memset (self->buffer, fillupwith, size);
  
  self->buffer[size] = 0;    
  
  return self;    
}

//----------------------------------------------------------------------------------------

EcBuffer ecbuf_create_str (EcString* ps)
{
  EcBuffer self = ENTC_NEW (EcBuffer_s);
  
  EcString s = *ps;
  
  self->size = strlen (s);
  self->buffer = (unsigned char*)s;
  
  *ps = NULL;
  
  return self;  
}

//----------------------------------------------------------------------------------------

EcBuffer ecbuf_create_uuid ()
{
/*
#ifdef _WIN32

  TGUID guid;

  if (CreateGUID (guid) == 0)
  {
    EcString uuid = GUIDToString (guid);

    return ecbuf_create_str (&uuid);  
  }

#else
*/      
  EcBuffer self = ecbuf_create (40);
  int t = 0;

  char *szTemp = "xxxxxxxx-xxxx-4xxx-yxxx-xxxxxxxxxxxx";
  char *szHex = "0123456789ABCDEF-";
  int nLen = strlen (szTemp);
  unsigned char* pos = self->buffer;
  
  srand (clock());

  for (t = 0; t < nLen + 1; t++, pos++)
  {
    int r = rand () % 16;
    char c = ' ';   
    
    switch (szTemp[t])
    {
      case 'x' : { c = szHex [r]; } break;
      case 'y' : { c = szHex [(r & 0x03) | 0x08]; } break;
      case '-' : { c = '-'; } break;
      case '4' : { c = '4'; } break;
    }
    
    *pos = ( t < nLen ) ? c : 0x00;
  }
      
      
//#endif
  return self;
}

//----------------------------------------------------------------------------------------

EcBuffer ecbuf_create_fromBuffer (const unsigned char* src, uint_t size)
{
  EcBuffer self = ecbuf_create (size);
  
  // copy content
  memcpy (self->buffer, src, size);
  
  return self;
}

//----------------------------------------------------------------------------------------

void ecbuf_destroy (EcBuffer* pself)
{
  EcBuffer self = *pself;

  memset (self->buffer, 0x00, self->size);
  ENTC_FREE (self->buffer);
  
  // delete buffer struct
  ENTC_DEL (pself, EcBuffer_s);  
}

//----------------------------------------------------------------------------------------

void ecbuf_setTerm (EcBuffer self, uint_t size)
{
  if (size <= self->size)
  {
    // set c-string termination
    self->buffer[size] = 0;
  }
  else
  {
    self->buffer[self->size] = 0;    
  }
}

//----------------------------------------------------------------------------------------

void ecbuf_fill (EcBuffer self, uint_t size, char fillupwith)
{
  uint_t fillup = size < self->size ? size : self->size;
  
  memset(self->buffer, fillupwith, fillup);
  
  self->buffer[fillup] = 0;    
}

//----------------------------------------------------------------------------------------

void ecbuf_random (EcBuffer self, uint_t size)
{
  uint_t fillup = size < self->size ? size : self->size;
  uint_t i;
  
  for(i = 0; i < fillup; i++)
  {
    (self->buffer)[i] = (rand() % 26) + 97;
  }
  (self->buffer)[i] = 0;    
}

//----------------------------------------------------------------------------------------

void ecbuf_format (EcBuffer self, uint_t size, const char* format, ...)
{
  va_list ptr;  
  va_start(ptr, format);
#ifdef _WIN32
  vsnprintf_s( (char*)self->buffer, self->size, size, format, ptr );
#elif __DOS__
  vsprintf((char*)self->buffer, format, ptr);
#else
  vsnprintf((char*)self->buffer, self->size, format, ptr );
#endif
  va_end(ptr);    
}

//----------------------------------------------------------------------------------------

void ecbuf_resize (EcBuffer self, uint_t size)
{
  self->buffer = (unsigned char*) realloc (self->buffer, size);
  
  if (self->buffer == NULL)
  {
    printf("******** FATAL: realloc failed !! **********\n");
  }
  
  self->size = size; 
}

//----------------------------------------------------------------------------------------

const EcString ecbuf_const_str (const EcBuffer self)
{
  return (const EcString)self->buffer;  
}

//----------------------------------------------------------------------------------------

EcString ecbuf_str (EcBuffer* pself)
{
  EcBuffer self = *pself;
  // transform buffer content into string
  EcString ret = (char*)self->buffer;
  // delete buffer struct
  ENTC_DEL (pself, EcBuffer_s);
  // return string
  return ret;  
}

//----------------------------------------------------------------------------------------

void ecbuf_replace (EcString* s, EcBuffer* pself)
{
  ecstr_replace (s, ecbuf_str (pself));
}

//----------------------------------------------------------------------------------------

#ifdef _WIN32

#include <windows.h>
#include <wincrypt.h>
#pragma comment (lib, "Crypt32.lib")

EcBuffer ecbuf_encode_base64 (EcBuffer self)
{
  EcBuffer ret = ENTC_NEW (EcBuffer_s);

  // calculate the size of the buffer
  CryptBinaryToString (self->buffer, self->size, CRYPT_STRING_BASE64 | CRYPT_STRING_NOCRLF, NULL, &(ret->size));

  // allocate the buffer
  ret->buffer = (unsigned char*)ENTC_MALLOC (ret->size + 1);

  // encrypt
  CryptBinaryToString (self->buffer, self->size, CRYPT_STRING_BASE64 | CRYPT_STRING_NOCRLF, ret->buffer, &(ret->size));
  ret->buffer [ret->size] = 0; 

  return ret;
}  

//----------------------------------------------------------------------------------------

EcBuffer ecbuf_decode_base64 (EcBuffer self)
{
  EcBuffer ret = ENTC_NEW (EcBuffer_s);

  // calculate the size of the buffer
  CryptStringToBinary (self->buffer, 0, CRYPT_STRING_BASE64, NULL, &(ret->size), NULL, NULL); 

  // allocate the buffer
  ret->buffer = (unsigned char*)ENTC_MALLOC (ret->size + 1);

  // decrypt
  CryptStringToBinary (self->buffer, 0, CRYPT_STRING_BASE64, ret->buffer, &(ret->size), NULL, NULL);
  ret->buffer [ret->size] = 0; 

  return ret; 
}

//----------------------------------------------------------------------------------------

ulong_t ecbuf_encode_base64_d (EcBuffer source, EcBuffer base64)
{
  ulong_t size;

  // decrypt
  CryptStringToBinary (source->buffer, 0, CRYPT_STRING_BASE64, base64->buffer, &size, NULL, NULL);
  base64->buffer [size] = 0; 

  return size; 
}

//----------------------------------------------------------------------------------------

ulong_t ecbuf_encode_base64_calculateSize (ulong_t max)
{
  return ((max - 1) / 4 * 3);
}

#else

//----------------------------------------------------------------------------------------

static const unsigned char pr2six[256] =
{
  /* ASCII table */
  64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
  64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
  64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 62, 64, 64, 64, 63,
  52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 64, 64, 64, 64, 64, 64,
  64,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14,
  15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 64, 64, 64, 64, 64,
  64, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40,
  41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 64, 64, 64, 64, 64,
  64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
  64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
  64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
  64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
  64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
  64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
  64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
  64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64
};

//----------------------------------------------------------------------------------------

int ecbuf_decode_base64_len (const char *bufcoded)
{
  int nbytesdecoded;
  register const unsigned char *bufin;
  register int nprbytes;
  
  bufin = (const unsigned char *) bufcoded;
  while (pr2six[*(bufin++)] <= 63);
  
  nprbytes = (bufin - (const unsigned char *) bufcoded) - 1;
  nbytesdecoded = ((nprbytes + 3) / 4) * 3;
  
  return nbytesdecoded + 1;
}

//----------------------------------------------------------------------------------------

EcBuffer ecbuf_decode_base64 (EcBuffer self)
{
  int nbytesdecoded;
  register const unsigned char *bufin;
  unsigned char* bufout;
  register int nprbytes;
  
  EcBuffer ret = ENTC_NEW (EcBuffer_s);
  
  bufin = (const unsigned char *) self->buffer;
  while (pr2six[*(bufin++)] <= 63);
  nprbytes = (bufin - (const unsigned char *) self->buffer) - 1;
  nbytesdecoded = ((nprbytes + 3) / 4) * 3;
  
  ret->buffer = (unsigned char*)ENTC_MALLOC (self->size);
  
  bufout = ret->buffer;  
  bufin = (const unsigned char *) self->buffer;
  
  while (nprbytes > 4)
  {
    *(bufout++) =
    (unsigned char) (pr2six[*bufin] << 2 | pr2six[bufin[1]] >> 4);
    *(bufout++) =
    (unsigned char) (pr2six[bufin[1]] << 4 | pr2six[bufin[2]] >> 2);
    *(bufout++) =
    (unsigned char) (pr2six[bufin[2]] << 6 | pr2six[bufin[3]]);
    bufin += 4;
    nprbytes -= 4;
  }
  
  /* Note: (nprbytes == 1) would be an error, so just ingore that case */
  if (nprbytes > 1)
  {
    *(bufout++) =
    (unsigned char) (pr2six[*bufin] << 2 | pr2six[bufin[1]] >> 4);
  }
  if (nprbytes > 2)
  {
    *(bufout++) =
    (unsigned char) (pr2six[bufin[1]] << 4 | pr2six[bufin[2]] >> 2);
  }
  if (nprbytes > 3)
  {
    *(bufout++) =
    (unsigned char) (pr2six[bufin[2]] << 6 | pr2six[bufin[3]]);
  }
  
  *(bufout++) = '\0';
  nbytesdecoded -= (4 - nprbytes) & 3;
  
  ret->buffer = realloc (ret->buffer, nbytesdecoded);  
  ret->size = nbytesdecoded;
    
  return ret;    
}

//----------------------------------------------------------------------------------------

static const char basis_64[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

int ecbuf_encode_base64_len (int len)
{
  return ((len + 2) / 3 * 4) + 1;
}

//----------------------------------------------------------------------------------------

ulong_t ecbuf_encode_base64_calculateSize (ulong_t max)
{
  return ((max - 1) / 4 * 3);
}

//----------------------------------------------------------------------------------------

ulong_t ecbuf_encode_base64_d (EcBuffer source, EcBuffer base64)
{
  int i;
  int l = source->size;
  ulong_t b64size = ecbuf_encode_base64_len (l);
  
  if (base64->size < b64size)
  {
    return 0;
  }

  unsigned char* b = base64->buffer;
  
  //unsigned char* b = (unsigned char*)ENTC_MALLOC(ecbuf_encode_base64_len (self->size));
  unsigned char* s = source->buffer;
  
  unsigned char* p = b;
  
  for (i = 0; i < l - 2; i += 3) {
    *p++ = basis_64[(s[i] >> 2) & 0x3F];
    *p++ = basis_64[((s[i] & 0x3) << 4) |
                    ((int) (s[i + 1] & 0xF0) >> 4)];
    *p++ = basis_64[((s[i + 1] & 0xF) << 2) |
                    ((int) (s[i + 2] & 0xC0) >> 6)];
    *p++ = basis_64[s[i + 2] & 0x3F];
  }
  if (i < l) {
    *p++ = basis_64[(s[i] >> 2) & 0x3F];
    if (i == (l - 1))
    {
      *p++ = basis_64[((s[i] & 0x3) << 4)];
      *p++ = '=';
    }
    else
    {
      *p++ = basis_64[((s[i] & 0x3) << 4) | ((int) (s[i + 1] & 0xF0) >> 4)];
      *p++ = basis_64[((s[i + 1] & 0xF) << 2)];
    }
    *p++ = '=';
  }
  
  return p - b;
}

//----------------------------------------------------------------------------------------

EcBuffer ecbuf_encode_base64 (EcBuffer self)
{
  EcBuffer ret;
  
  int i;
  int l = self->size;
  unsigned char* b = (unsigned char*)ENTC_MALLOC(ecbuf_encode_base64_len (self->size));
  unsigned char* s = self->buffer;
  
  unsigned char* p = b;
  
  for (i = 0; i < l - 2; i += 3) {
    *p++ = basis_64[(s[i] >> 2) & 0x3F];
    *p++ = basis_64[((s[i] & 0x3) << 4) |
                    ((int) (s[i + 1] & 0xF0) >> 4)];
    *p++ = basis_64[((s[i + 1] & 0xF) << 2) |
                    ((int) (s[i + 2] & 0xC0) >> 6)];
    *p++ = basis_64[s[i + 2] & 0x3F];
  }
  if (i < l) {
    *p++ = basis_64[(s[i] >> 2) & 0x3F];
    if (i == (l - 1))
    {
      *p++ = basis_64[((s[i] & 0x3) << 4)];
      *p++ = '=';
    }
    else
    {
      *p++ = basis_64[((s[i] & 0x3) << 4) | ((int) (s[i + 1] & 0xF0) >> 4)];
      *p++ = basis_64[((s[i + 1] & 0xF) << 2)];
    }
    *p++ = '=';
  }
  
  *p++ = '\0';
  
  ret = ENTC_NEW (EcBuffer_s);
    
  ret->size = p - b;
  ret->buffer = (unsigned char*)realloc (b, ret->size);

  return ret;    
}

//----------------------------------------------------------------------------------------

#endif

//----------------------------------------------------------------------------------------

EcBuffer ecbuf_md5 (EcBuffer self)
{
  EcBuffer ret;
  unsigned char digest[16];
  // creates the digest as md5 checksum
  {
    int l = self->size;
    const unsigned char* str = self->buffer;
    
    md5_state_t state;
    
    md5_init(&state);    
    // do it in chunks if buffer is bigger than 512
    while (l > 0)
    {
      if (l > 512)
      {
        md5_append(&state, str, 512);
      }
      else
      {
        md5_append(&state, str, l);
      }
      l -= 512;
      str += 512;
    }
    md5_finish(&state, digest);
  }
    
  // creates a buffer with 32 chars + 1 extra for null termination
  ret = ecbuf_create (32);

  // fast conversion from digits to hex string
  {
    static char hex [] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9' ,'a', 'b', 'c', 'd', 'e', 'f' };
    int n;
    
    unsigned char* pos = ret->buffer;
    for (n = 0; n < 16; n++)
    {
      unsigned char h = digest[n];
      // higher 4 bits
      *pos = hex[h >> 4];  // transform into readable char
      pos++;
      // lower 4 bits
      *pos = hex[h &0xf];  // transform into readable char
      pos++;
    } 
  }
   
  return ret;    
}

//----------------------------------------------------------------------------------------

typedef struct
{ 

  unsigned char data[64]; 
  uint_t datalen; 
  uint_t bitlen[2]; 
  uint_t state[5]; 
  uint_t k[4]; 

} ENTC_SHA1; 

//----------------------------------------------------------------------------------------

void ecbuf_sha1_init (ENTC_SHA1 *ctx) 
{  
  ctx->datalen = 0; 
  ctx->bitlen[0] = 0; 
  ctx->bitlen[1] = 0; 
  ctx->state[0] = 0x67452301; 
  ctx->state[1] = 0xEFCDAB89; 
  ctx->state[2] = 0x98BADCFE; 
  ctx->state[3] = 0x10325476; 
  ctx->state[4] = 0xc3d2e1f0; 
  ctx->k[0] = 0x5a827999; 
  ctx->k[1] = 0x6ed9eba1; 
  ctx->k[2] = 0x8f1bbcdc; 
  ctx->k[3] = 0xca62c1d6; 
}  

// DBL_INT_ADD treats two unsigned ints a and b as one 64-bit integer and adds c to it
#define ROTLEFT(a,b) ((a << b) | (a >> (32-b))) 
#define DBL_INT_ADD(a,b,c) if (a > 0xffffffff - c) ++b; a += c; 

//----------------------------------------------------------------------------------------

void ecbuf_sha1_trans (ENTC_SHA1 *ctx, unsigned char data[]) 
{  
  uint_t a,b,c,d,e,i,j,t,m[80]; 
  
  for (i = 0, j = 0; i < 16; ++i, j += 4) 
  {
    m[i] = (data[j] << 24) + (data[j+1] << 16) + (data[j+2] << 8) + (data[j+3]); 
  }
  
  for ( ; i < 80; ++i)
  { 
    m[i] = (m[i-3] ^ m[i-8] ^ m[i-14] ^ m[i-16]); 
    m[i] = (m[i] << 1) | (m[i] >> 31); 
  }  
  
  a = ctx->state[0]; 
  b = ctx->state[1]; 
  c = ctx->state[2]; 
  d = ctx->state[3]; 
  e = ctx->state[4]; 
  
  for (i=0; i < 20; ++i)
  { 
    t = ROTLEFT(a,5) + ((b & c) ^ (~b & d)) + e + ctx->k[0] + m[i]; 
    e = d; 
    d = c; 
    c = ROTLEFT(b,30); 
    b = a; 
    a = t; 
  }  
  for ( ; i < 40; ++i)
  { 
    t = ROTLEFT(a,5) + (b ^ c ^ d) + e + ctx->k[1] + m[i]; 
    e = d; 
    d = c; 
    c = ROTLEFT(b,30); 
    b = a; 
    a = t; 
  }  
  for ( ; i < 60; ++i)
  { 
    t = ROTLEFT(a,5) + ((b & c) ^ (b & d) ^ (c & d))  + e + ctx->k[2] + m[i]; 
    e = d; 
    d = c; 
    c = ROTLEFT(b,30); 
    b = a; 
    a = t; 
  }  
  for ( ; i < 80; ++i)
  { 
    t = ROTLEFT(a,5) + (b ^ c ^ d) + e + ctx->k[3] + m[i]; 
    e = d; 
    d = c; 
    c = ROTLEFT(b,30); 
    b = a; 
    a = t; 
  }  
  
  ctx->state[0] += a; 
  ctx->state[1] += b; 
  ctx->state[2] += c; 
  ctx->state[3] += d; 
  ctx->state[4] += e; 
}  

//----------------------------------------------------------------------------------------

void ecbuf_sha1_set (ENTC_SHA1* ctx, unsigned char data[], uint_t len) 
{  
  uint_t i;
  for (i = 0; i < len; ++i)
  { 
    ctx->data[ctx->datalen] = data[i]; 
    ctx->datalen++; 
    if (ctx->datalen == 64)
    { 
      ecbuf_sha1_trans (ctx, ctx->data); 
      DBL_INT_ADD(ctx->bitlen[0], ctx->bitlen[1], 512); 
      ctx->datalen = 0; 
    }  
  }  
} 

//----------------------------------------------------------------------------------------

void ecbuf_sha1_final (ENTC_SHA1* ctx, unsigned char hash[]) 
{  
  uint_t i; 
  
  i = ctx->datalen; 
  
  // Pad whatever data is left in the buffer. 
  if (ctx->datalen < 56)
  { 
    ctx->data[i++] = 0x80; 
    while (i < 56)
    {
      ctx->data[i++] = 0x00; 
    }
  }  
  else 
  { 
    ctx->data[i++] = 0x80; 
    while (i < 64)
    {
      ctx->data[i++] = 0x00; 
    }
    ecbuf_sha1_trans (ctx, ctx->data); 
    memset(ctx->data,0,56); 
  }  
  
  // Append to the padding the total message's length in bits and transform. 
  DBL_INT_ADD(ctx->bitlen[0],ctx->bitlen[1],8 * ctx->datalen); 
  ctx->data[63] = ctx->bitlen[0]; 
  ctx->data[62] = ctx->bitlen[0] >> 8; 
  ctx->data[61] = ctx->bitlen[0] >> 16; 
  ctx->data[60] = ctx->bitlen[0] >> 24; 
  ctx->data[59] = ctx->bitlen[1]; 
  ctx->data[58] = ctx->bitlen[1] >> 8; 
  ctx->data[57] = ctx->bitlen[1] >> 16;  
  ctx->data[56] = ctx->bitlen[1] >> 24; 
  ecbuf_sha1_trans (ctx, ctx->data); 
  
  // Since this implementation uses little endian byte ordering and MD uses big endian, 
  // reverse all the bytes when copying the final state to the output hash. 
  for (i = 0; i < 4; ++i)
  { 
    hash[i]    = (ctx->state[0] >> (24-i*8)) & 0x000000ff; 
    hash[i+4]  = (ctx->state[1] >> (24-i*8)) & 0x000000ff; 
    hash[i+8]  = (ctx->state[2] >> (24-i*8)) & 0x000000ff; 
    hash[i+12] = (ctx->state[3] >> (24-i*8)) & 0x000000ff; 
    hash[i+16] = (ctx->state[4] >> (24-i*8)) & 0x000000ff; 
  }  
}  

//----------------------------------------------------------------------------------------

EcBuffer ecbuf_sha1 (EcBuffer self)
{
  ENTC_SHA1 sha1;
  EcBuffer ret = ecbuf_create (20);
  
  ecbuf_sha1_init (&sha1);
  
  ecbuf_sha1_set (&sha1, self->buffer, self->size);
  
  ecbuf_sha1_final (&sha1, ret->buffer);
  
  return ret;
}

//----------------------------------------------------------------------------------------

#ifdef _WIN32

EcBuffer ecbuf_sha_256 (EcBuffer b1)
{
  EcBuffer ret = NULL;

  HCRYPTPROV provHandle = (HCRYPTPROV)NULL;
  HCRYPTHASH hashHandle;
  
  if (!CryptAcquireContext (&provHandle, NULL, NULL, PROV_RSA_AES, 0))
  {
    return NULL;
  }
  
  if (!CryptCreateHash (provHandle, CALG_SHA_256, 0, 0, &hashHandle))
  {
    CryptReleaseContext (provHandle, 0);
    
    return NULL;
  }
  
  {
    EcStream stream = ecstream_new ();

    BYTE hash [32];
    DWORD hashlen;
  
    if (CryptHashData (hashHandle, b1->buffer, b1->size, 0) && CryptGetHashParam (hashHandle, HP_HASHVAL, hash, &hashlen, 0))
    {
      CHAR rgbDigits[] = "0123456789abcdef";
      DWORD i;    

      for (i = 0; i < hashlen; i++)
      {
        ecstream_appendc (stream, (char)(rgbDigits[hash[i] >> 4]));
        ecstream_appendc (stream, (char)(rgbDigits[hash[i] & 0xf]));
      }

      ret = ecstream_trans (&stream);
    }
  }
  
  CryptDestroyHash (hashHandle);
  CryptReleaseContext (provHandle, 0);
  
  return ret;
}

//----------------------------------------------------------------------------------------

#else

#include <openssl/sha.h>

EcBuffer ecbuf_sha_256 (EcBuffer b1)
{
  unsigned char hash [SHA256_DIGEST_LENGTH];
  EcBuffer outb = ecbuf_create (64);
  
  SHA256_CTX sha256;

  SHA256_Init (&sha256);
  
  SHA256_Update (&sha256, b1->buffer, b1->size);
  
  SHA256_Final(hash, &sha256);
  
  int i = 0;
  for(i = 0; i < SHA256_DIGEST_LENGTH; i++)
  {
    sprintf((char*)(outb->buffer) + (i * 2), "%02x", hash[i]);
  }

  return outb;
}

#endif

//----------------------------------------------------------------------------------------

EcBuffer ecbuf_xor (EcBuffer b1, EcBuffer b2)
{
  uint_t i;
  uint_t m = ENTC_MIN (b1->size, b2->size);
  
  EcBuffer ret = ecbuf_create (m);
  
  for (i = 0; i < m; i++)
  {
    // xor
    ret->buffer [i] = b1->buffer [i] ^ b2->buffer [i];
  }
  
  return ret;
}

//----------------------------------------------------------------------------------------

EcBuffer ecbuf_concat (EcBuffer b1, EcBuffer b2)
{
  EcBuffer ret = ecbuf_create (b1->size + b2->size);
  
  memcpy (ret->buffer, b1->buffer, b1->size);
  memcpy (ret->buffer + b1->size, b2->buffer, b2->size);
  
  return ret;
}

//----------------------------------------------------------------------------------------

void ecbuf_iterator (EcBuffer self, EcBufferIterator* bufit)
{
  bufit->buf = self;
  bufit->line = ecstr_init ();

  ecbufit_reset (bufit);
}

//----------------------------------------------------------------------------------------

int ecbufit_readln_2breaks (EcBufferIterator* bufit, EcStream stream)
{
  int hasBreak1 = FALSE;
  
  while (*(bufit->pos))
  {
    char c = *(bufit->pos);
    
    if (hasBreak1)
    {
      if (c == bufit->b2)
      {
        bufit->pos++;
        return TRUE;
      }
      else
      {        
        bufit->pos++;
        return TRUE;
      }
    }
    else
    {
      if (c == bufit->b1)
      {
        hasBreak1 = TRUE;
      }
      else if (c == '\r' || c == '\n')
      {
        bufit->pos++;
        // error, but we count it as line break
        return TRUE;
      }
      else
      {
        ecstream_append_c (stream, c);
      }                  
    }
    
    bufit->pos++;
  }
  
  return FALSE;  
}

//----------------------------------------------------------------------------------------

int ecbufit_readln_1breaks (EcBufferIterator* bufit, EcStream stream)
{
  while (*(bufit->pos))
  {
    char c = *(bufit->pos);
    
    if (c == bufit->b1)
    {
      bufit->pos++;
      return TRUE;
    }
    else if (c == '\r' || c == '\n')
    {
      bufit->pos++;
      // error, but we count it as line break
      return TRUE;
    }
    else
    {
      ecstream_append_c (stream, c);
    } 
    
    bufit->pos++;
  }
  
  return FALSE;    
}

//----------------------------------------------------------------------------------------

int ecbufit_readln_getbreaks (EcBufferIterator* bufit, EcStream stream)
{
  int hasBreak1 = FALSE;
  
  while (*(bufit->pos))
  {
    char c = *(bufit->pos);
    
    if (hasBreak1)
    {
      if (c == bufit->b1)
      {
        bufit->pos++;
        return TRUE;
      }
      else if (c == '\r' || c == '\n')
      {
        bufit->b2 = c;
        bufit->pos++;
        return TRUE;
      }
      else
      {
        bufit->pos++;
        return TRUE;
      }
    }
    else
    {
      if (c == '\r' || c == '\n')
      {
        bufit->b1 = c;
        hasBreak1 = TRUE;

        bufit->pos++;
        
        continue;
      }
      else
      {
        ecstream_append_c (stream, c);
      }                  
    }
    
    bufit->pos++;
  }
  
  return FALSE;      
}

//----------------------------------------------------------------------------------------

int ecbufit_readln (EcBufferIterator* bufit)
{
  EcStream stream = ecstream_create ();
  int res;
  
  if (bufit->b1 && bufit->b2)
  {
    res = ecbufit_readln_2breaks (bufit, stream);
  }
  else if (bufit->b1 && (bufit->b2 == 0))
  {
    res = ecbufit_readln_1breaks (bufit, stream);
  }
  else
  {
    res = ecbufit_readln_getbreaks (bufit, stream);
  } 

  {  
    EcBuffer buf = ecstream_tobuf (&stream);
    ecbuf_replace (&(bufit->line), &buf);
  } 
  return res;
}

//----------------------------------------------------------------------------------------

void ecbufit_reset (EcBufferIterator* bufit)
{
  ecstr_delete (&(bufit->line));
  
  bufit->b1 = 0;
  bufit->b2 = 0;
  
  bufit->pos = bufit->buf->buffer;  
}

//----------------------------------------------------------------------------------------

EcBuffer ecbuf_hex2bin (EcBuffer hex)
{
  uint_t blen = hex->size;
  
  uint8_t  posHex = 0;
  uint8_t  posBin = 0;
  
  uint8_t  idx0;
  uint8_t  idx1;
  
  EcBuffer bin = ecbuf_create (blen / 2);
  
  // mapping of ASCII characters to hex values
  const uint8_t hashmap[] =
  {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // ........
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // ........
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // ........
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // ........
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, //  !"#$%&'
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // ()*+,-./
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, // 01234567
    0x08, 0x09, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 89:;<=>?
    0x00, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x00, // @ABCDEFG
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // HIJKLMNO
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // PQRSTUVW
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // XYZ[\]^_
    0x00, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x00, // `abcdefg
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // hijklmno
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // pqrstuvw
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // xyz{|}~.
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // ........
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // ........
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // ........
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // ........
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // ........
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // ........
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // ........
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // ........
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // ........
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // ........
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // ........
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // ........
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // ........
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // ........
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // ........
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00  // ........
  };
  
  for (; ((posBin < bin->size) && (posHex < hex->size)); posBin++, posHex += 2)
  {
    idx0 = (uint8_t)hex->buffer[posHex+0];
    idx1 = (uint8_t)hex->buffer[posHex+1];
    bin->buffer[posBin] = (uint8_t)(hashmap[idx0] << 4) | hashmap[idx1];
  }
  
  return bin;
}

//----------------------------------------------------------------------------------------

EcBuffer ecbuf_bin2hex (EcBuffer bin)
{
  const char hexmap [256][2] = {
    {'0','0'},{'0','1'},{'0','2'},{'0','3'},{'0','4'},{'0','5'},{'0','6'},{'0','7'},
    {'0','8'},{'0','9'},{'0','a'},{'0','b'},{'0','c'},{'0','d'},{'0','e'},{'0','f'},
    {'1','0'},{'1','1'},{'1','2'},{'1','3'},{'1','4'},{'1','5'},{'1','6'},{'1','7'},
    {'1','8'},{'1','9'},{'1','a'},{'1','b'},{'1','c'},{'1','d'},{'1','e'},{'1','f'},
    {'2','0'},{'2','1'},{'2','2'},{'2','3'},{'2','4'},{'2','5'},{'2','6'},{'2','7'},
    {'2','8'},{'2','9'},{'2','a'},{'2','b'},{'2','c'},{'2','d'},{'2','e'},{'2','f'},
    {'3','0'},{'3','1'},{'3','2'},{'3','3'},{'3','4'},{'3','5'},{'3','6'},{'3','7'},
    {'3','8'},{'3','9'},{'3','a'},{'3','b'},{'3','c'},{'3','d'},{'3','e'},{'3','f'},
    {'4','0'},{'4','1'},{'4','2'},{'4','3'},{'4','4'},{'4','5'},{'4','6'},{'4','7'},
    {'4','8'},{'4','9'},{'4','a'},{'4','b'},{'4','c'},{'4','d'},{'4','e'},{'4','f'},
    {'5','0'},{'5','1'},{'5','2'},{'5','3'},{'5','4'},{'5','5'},{'5','6'},{'5','7'},
    {'5','8'},{'5','9'},{'5','a'},{'5','b'},{'5','c'},{'5','d'},{'5','e'},{'5','f'},
    {'6','0'},{'6','1'},{'6','2'},{'6','3'},{'6','4'},{'6','5'},{'6','6'},{'6','7'},
    {'6','8'},{'6','9'},{'6','a'},{'6','b'},{'6','c'},{'6','d'},{'6','e'},{'6','f'},
    {'7','0'},{'7','1'},{'7','2'},{'7','3'},{'7','4'},{'7','5'},{'7','6'},{'7','7'},
    {'7','8'},{'7','9'},{'7','a'},{'7','b'},{'7','c'},{'7','d'},{'7','e'},{'7','f'},
    {'8','0'},{'8','1'},{'8','2'},{'8','3'},{'8','4'},{'8','5'},{'8','6'},{'8','7'},
    {'8','8'},{'8','9'},{'8','a'},{'8','b'},{'8','c'},{'8','d'},{'8','e'},{'8','f'},
    {'9','0'},{'9','1'},{'9','2'},{'9','3'},{'9','4'},{'9','5'},{'9','6'},{'9','7'},
    {'9','8'},{'9','9'},{'9','a'},{'9','b'},{'9','c'},{'9','d'},{'9','e'},{'9','f'},
    {'a','0'},{'a','1'},{'a','2'},{'a','3'},{'a','4'},{'a','5'},{'a','6'},{'a','7'},
    {'a','8'},{'a','9'},{'a','a'},{'a','b'},{'a','c'},{'a','d'},{'a','e'},{'a','f'},
    {'b','0'},{'b','1'},{'b','2'},{'b','3'},{'b','4'},{'b','5'},{'b','6'},{'b','7'},
    {'b','8'},{'b','9'},{'b','a'},{'b','b'},{'b','c'},{'b','d'},{'b','e'},{'b','f'},
    {'c','0'},{'c','1'},{'c','2'},{'c','3'},{'c','4'},{'c','5'},{'c','6'},{'c','7'},
    {'c','8'},{'c','9'},{'c','a'},{'c','b'},{'c','c'},{'c','d'},{'c','e'},{'c','f'},
    {'d','0'},{'d','1'},{'d','2'},{'d','3'},{'d','4'},{'d','5'},{'d','6'},{'d','7'},
    {'d','8'},{'d','9'},{'d','a'},{'d','b'},{'d','c'},{'d','d'},{'d','e'},{'d','f'},
    {'e','0'},{'e','1'},{'e','2'},{'e','3'},{'e','4'},{'e','5'},{'e','6'},{'e','7'},
    {'e','8'},{'e','9'},{'e','a'},{'e','b'},{'e','c'},{'e','d'},{'e','e'},{'e','f'},
    {'f','0'},{'f','1'},{'f','2'},{'f','3'},{'f','4'},{'f','5'},{'f','6'},{'f','7'},
    {'f','8'},{'f','9'},{'f','a'},{'f','b'},{'f','c'},{'f','d'},{'f','e'},{'f','f'}
  };

  EcBuffer hex = ecbuf_create (bin->size + bin->size + 1);

  uint32_t pos;
  for (pos = 0; pos < bin->size; pos++)
  {
    hex->buffer[pos + pos] = hexmap[*(bin->buffer + pos)][0];
    hex->buffer[pos + pos + 1] = hexmap[*(bin->buffer + pos)][1];
  }
  
  return hex;
}

//----------------------------------------------------------------------------------------
