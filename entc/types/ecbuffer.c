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
  
  ret->buffer = realloc (ret->buffer, nbytesdecoded + 1);
  ret->buffer [nbytesdecoded] = 0;
  
  ret->size = nbytesdecoded + 1;
    
  return ret;    
}

//----------------------------------------------------------------------------------------

static const char basis_64[] =
"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

int ecbuf_encode_base64_len (int len)
{
  return ((len + 2) / 3 * 4) + 1;
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
        ecstream_appendc (stream, c);      
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
      ecstream_appendc (stream, c);      
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
        ecstream_appendc (stream, c);      
      }                  
    }
    
    bufit->pos++;
  }
  
  return FALSE;      
}

//----------------------------------------------------------------------------------------

int ecbufit_readln (EcBufferIterator* bufit)
{
  EcStream stream = ecstream_new ();
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
    EcBuffer buf = ecstream_trans (&stream);
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
