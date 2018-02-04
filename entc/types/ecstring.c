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

#include "ecstring.h"

#include "ecbuffer.h"
#include "ecstream.h"

#include <string.h>
#include <stdarg.h>
#include <stdlib.h>
#include <ctype.h>

#ifdef _WIN32

#include <windows.h>
#include <stdio.h>

#else

#define _GNU_SOURCE
#include <stdio.h>

#endif

/*------------------------------------------------------------------------*/

EcString ecstr_init()
{
  return 0;  
}

/*------------------------------------------------------------------------*/

int ecstr_valid( const EcString s )
{
  if( s == 0 )
  {
    return FALSE;
  }
  else
  {
    return TRUE;
  }
}

/*------------------------------------------------------------------------*/

EcString ecstr_copy( const EcString source )
{
  /* check if we have at least value */
  if( !ecstr_valid( source ) )
  {
    return ecstr_init();
  }  
  /* get the buffer */
#ifdef _WIN32
  return _strdup( source );  
#elif __DOS__
  {
    EcString res = (char*)QMALLOC(strlen(source) + 1);
    strcpy(res, source);
    return res;
  }
#else
  return strdup( source );
#endif
}

/*------------------------------------------------------------------------*/

EcString ecstr_part (const EcString source, uint_t length)
{
  char* ret;
  
  /* check if we have at least value */
  if( !ecstr_valid( source ) )
  {
    return ecstr_init();
  }  
  
  ret = (char*)ENTC_MALLOC( (2 + length) * sizeof(char) );
  /* copy the part */
#ifdef _WIN32
  memcpy(ret, source, length * sizeof(char));
#else
  strncpy(ret, source, length);
#endif
  /* set the termination */
  ret[length] = 0;
  
  return ret;  
}

//----------------------------------------------------------------------------------------

EcString ecstr_format_list (const EcString format, va_list ptr)
{
  char* ret = NULL;

#ifdef _WIN32
  {
    int len = 1024;
    ret = (char*)ENTC_MALLOC(len);

    vsnprintf_s (ret, len, len, format, ptr);
  }
#else
  vasprintf (&ret, format, ptr);
#endif

  return ret;
}

//----------------------------------------------------------------------------------------

EcString ecstr_format (const EcString format, ...)
{
  EcString ret = NULL;

  va_list ptr;  
  va_start(ptr, format);

  ret = ecstr_format_list (format, ptr);

  va_end(ptr); 

  return ret;
}

//----------------------------------------------------------------------------------------

EcString ecstr_long (uint64_t value)
{
  /* create buffer with size 12 */
  EcBuffer buffer = ecbuf_create (12);
  /* transform unsigned value into string */  
  ecbuf_format (buffer, 11, "%lu", value );
  /* transform buffer into string */
  return ecbuf_str (&buffer);
}

//----------------------------------------------------------------------------------------

EcString ecstr_longPadded ( uint64_t value, int amount)
{
  EcBuffer buffer = ecbuf_create (12 + amount);
  
  // generate the format string
  EcBuffer h = ecbuf_create (21);
  ecbuf_format (h, 20, "%%0%ilu", amount);

  // do formatting and padding
  ecbuf_format (buffer, 11 + amount, ecbuf_const_str(h), value);

  ecbuf_destroy (&h);
  return ecbuf_str (&buffer);
}

//----------------------------------------------------------------------------------------

EcString ecstr_float (double value, uint_t n)
{
  EcString s;
  if( n > 0 )
  {
	  EcBuffer buffer01 = ecbuf_create (7);
	  EcBuffer buffer02 = ecbuf_create (31);
    
    ecbuf_format (buffer01, 6, "%%.%uf", n);
    /* transform to integer */
    ecbuf_format (buffer02, 30, ecbuf_const_str (buffer01), value );
    /* copy */
    s = ecbuf_str (&buffer02);
	  ecbuf_destroy (&buffer01);
  }
  else
  {
	  EcBuffer buffer02 = ecbuf_create (31);
    /* transform to integer */
    ecbuf_format (buffer02, 30, "%f", value);
    /* copy */
	  s = ecbuf_str (&buffer02);
  }
  return s;
}

//----------------------------------------------------------------------------------------

EcString ecstr_filled (char c, uint_t n)
{
  EcString s = (EcString)malloc((n + 1));
  
  memset (s, c, n);
  s[n] = 0;
  
  return s;
}

/*------------------------------------------------------------------------*/

void ecstr_delete( EcString* ptr )
{
  EcString self = *ptr;

  if(self != NULL)
  {
    /* clear the whole string */
    memset(self, 0x00, strlen(self));
    /* clear memory */
    ENTC_FREE(self);
  }
  *ptr = NULL;
}

/*------------------------------------------------------------------------*/

EcString ecstr_replace( EcString* s, const EcString value )
{
  EcString s_new = ecstr_copy( value );
  
  ecstr_delete( s );
  
  *s = s_new;
  
  return *s;
}

/*------------------------------------------------------------------------*/

EcString ecstr_replaceTO( EcString* s, EcString value )
{
  ecstr_delete( s );
  
  *s = value;
  
  return *s;  
}

/*------------------------------------------------------------------------*/

EcString ecstr_replacePos( EcString* s, const EcString value, const EcString pos )
{
  ecstr_delete( s );
  
  if(pos > value)
  {
    uint_t diff = pos - value;
        
    *s = ecstr_part(value, diff);
    
    return *s;
  }
  
  return 0;
}

/*------------------------------------------------------------------------*/

EcString ecstr_cat2(const EcString s1, const EcString s2)
{
  if( !s1 )
  {
    return ecstr_copy(s2);  
  }
  
  if( !s2 )
  {
    return ecstr_copy(s1);  
  }
  
  {
    /* variables */
    int s1_len = strlen(s1);
    int s2_len = strlen(s2);
    
    char* ret;
    char* pos;
    
    ret = (char*)ENTC_MALLOC( (s1_len + s2_len + 10) );
    if (ret == NULL) 
    {
      return NULL;
    }
  
    pos = ret;
  
    memcpy(pos, s1, s1_len);
    pos = pos + s1_len;
  
    memcpy(pos, s2, s2_len);
    pos = pos + s2_len;
  
    *pos = 0;
  
    return ret;
  }
}

/*------------------------------------------------------------------------*/

EcString ecstr_catc(const EcString s1, char c, const EcString s2)
{
  // variables
  uint_t s1_len;
  uint_t s2_len;
  char* ret;
  char* pos;

  if (s1 == NULL) 
  {
    return NULL;
  }
  
  if (s2 == NULL) 
  {
    return NULL;
  }
  
  s1_len = strlen(s1);
  s2_len = strlen(s2);
  
  ret = (char*)ENTC_MALLOC( (s1_len + s2_len + 2) );
  
  if (ret == NULL) 
  {
    return NULL;
  }
  
  pos = ret;
  
  if (s1_len > 0) 
  {
    memcpy(ret, s1, s1_len);
    pos = pos + s1_len;
  }
  
  *pos = c;
  pos++;
  
  if (s2_len > 0)
  {
    memcpy(pos, s2, s2_len);
    pos = pos + s2_len;    
  }
  
  *pos = 0;
  
  return ret;  
}

/*------------------------------------------------------------------------*/

EcString ecstr_cat3(const EcString s1, const EcString s2, const EcString s3)
{
  uint_t s1_len = strlen(s1);
  uint_t s2_len = strlen(s2);
  uint_t s3_len = strlen(s3);
  
  char* ret = (char*)ENTC_MALLOC( (s1_len + s2_len + s3_len + 1) * sizeof(char) );
  
  char* pos = ret;
  
  memcpy(pos, s1, s1_len);
  pos = pos + s1_len;
  
  memcpy(pos, s2, s2_len);
  pos = pos + s2_len;
  
  memcpy(pos, s3, s3_len);
  pos = pos + s3_len;
  
  *pos = 0;
  
  return ret;  
}

/*------------------------------------------------------------------------*/

EcString ecstr_cat4(const EcString s1, const EcString s2, const EcString s3, const EcString s4)
{
  uint_t s1_len = strlen(s1);
  uint_t s2_len = strlen(s2);
  uint_t s3_len = strlen(s3);
  uint_t s4_len = strlen(s4);
  
  char* ret = (char*)ENTC_MALLOC( (s1_len + s2_len + s3_len + s4_len + 1) * sizeof(char) );
  
  char* pos = ret;
  
  memcpy(pos, s1, s1_len);
  pos = pos + s1_len;
  
  memcpy(pos, s2, s2_len);
  pos = pos + s2_len;
  
  memcpy(pos, s3, s3_len);
  pos = pos + s3_len;
  
  memcpy(pos, s4, s4_len);
  pos = pos + s4_len;
  
  *pos = 0;
  
  return ret;  
}

/*------------------------------------------------------------------------*/

uint_t ecstr_len (const EcString s)
{
  return strlen (s);
}

/*------------------------------------------------------------------------*/

const char* ecstr_cstring( const EcString s )
{
  if( s )
  {
    return s;
  }
  else
  {
    return "{NULL}";
  }
}

/*------------------------------------------------------------------------*/

int ecstr_equal( const EcString s1, const EcString s2 )
{
  if( !s1 )
  {
    return FALSE;  
  }
  
  if( !s2 )
  {
    return FALSE;  
  }
  
  return strcmp(s1, s2) == 0;
}

//---------------------------------------------------------------------------

int ecstr_equalUnsensitive (const EcString s1, const EcString s2)
{
  int ret;

  if( !s1 )
  {
    return FALSE;  
  }
  
  if( !s2 )
  {
    return FALSE;  
  }
  // TODO: make it better
  {
    EcString u1 = ecstr_copy (s1);
    EcString u2 = ecstr_copy (s2);

    ecstr_toUpper (u1);
    ecstr_toUpper (u2);

    ret = strcmp(u1, u2) == 0;

    ecstr_delete (&u1);
    ecstr_delete (&u2);
  }  
  return ret;
}

//---------------------------------------------------------------------------

int ecstr_equaln (const EcString s1, const EcString s2, uint_t size)
{
  if( !s1 )
  {
    return FALSE;  
  }
  
  if( !s2 )
  {
    return FALSE;  
  }

  return strncmp(s1, s2, size) == 0;
}

/*------------------------------------------------------------------------*/

int ecstr_leading (const EcString s, const EcString leading)
{
  if( !s )
  {
    return FALSE;  
  }
  
  if( !leading )
  {
    return FALSE;  
  }

  {
    int len = strlen(leading) - 1;
    
    return strncmp(s, leading, len) == 0;
  }
}

//----------------------------------------------------------------------------------------

int ecstr_leadingPart (const EcString source, const EcString leading, EcString* result)
{
  int len;
  
  if( !source )
  {
    return FALSE;  
  }
  
  if( !leading )
  {
    return FALSE;  
  }
  
  len = strlen(leading);
  
  if (strncmp(source, leading, len) != 0)
  {
    return FALSE;
  }
  
  if (result)
  {
    ecstr_replaceTO(result, ecstr_part(source + len, strlen(source) - len));
  }
  
  return TRUE;
}

/*------------------------------------------------------------------------*/

int ecstr_ending (const EcString s, const EcString ending)
{
  if( !s )
  {
    return FALSE;  
  }
  
  if( !ending )
  {
    return FALSE;  
  }

  {
    int lenEnding = strlen (ending);
    int lenSource = strlen (s);
    
    return strncmp(s + (lenSource - lenEnding), ending, lenEnding) == 0;    
  }
}

/*------------------------------------------------------------------------*/

int ecstr_endingPart (const EcString source, const EcString ending, EcString* result)
{
  int lenEnding;
  int lenSource;

  if( !source )
  {
    return FALSE;  
  }
  
  if( !ending )
  {
    return FALSE;  
  }
  
  lenEnding = strlen (ending);
  lenSource = strlen (source);
  
  if (strncmp(source + (lenSource - lenEnding), ending, lenEnding) != 0)
  {
    return FALSE;
  }
  
  ecstr_replaceTO(result, ecstr_part(source, lenSource - lenEnding));  
  return TRUE;
}

/*------------------------------------------------------------------------*/

int ecstr_has (const EcString self, char c)
{
  if( isNotAssigned (self))
  {
    return FALSE;  
  }
  
  return isAssigned (strchr (self, c));  
}

/*------------------------------------------------------------------------*/

EcString ecstr_trim( const EcString s )
{
  /* variables */
  EcString copy;
  char* pos01;
  char* pos02;

  if( !s )
  {
    return 0;  
  }
  
  copy = ecstr_copy( s );

  /* source position */
  pos01 = copy;
  pos02 = 0;
  /* trim from begin */
  while(*pos01)
  {
    if( (*pos01 > 32) && (*pos01 < 127) )
      break;
    
    pos01++;
  }
  pos02 = copy;
  /* copy rest */
  while(*pos01)
  {
    *pos02 = *pos01;
    
    pos01++;
    pos02++;
  }
  /* set here 0 not to run in undefined situation */
  *pos02 = 0;
  /* trim from the end */
  while( pos02 != copy )
  {
    /* decrease */
    pos02--;
    /* check if readable */
    if( (*pos02 > 32) && (*pos02 < 127) )
      break;
    /* set to zero */
    *pos02 = 0;
  }  
  
  return copy;
}

/*------------------------------------------------------------------------*/


EcString ecstr_trimNonePrintable( const EcString s )
{
  EcString copy = ecstr_copy( s );

  const char* pos_source = s;
  char* pos_dest = copy;
  while(*pos_source)
  {
    if(*pos_source > 31)
    {
      *pos_dest = *pos_source;
      pos_dest++;
    }
    pos_source++;
  }
  *pos_dest = 0;
  
  return copy;
}

//-------------------------------------------------------------------------------

EcString ecstr_trimc (const EcString s, char c)
{
  return ecstr_trimlr (s, c, c);
}

//-------------------------------------------------------------------------------

EcString ecstr_trimlr (const EcString s, char l, char r)
{
  /* variables */
  EcString copy;
  char* pos01;
  char* pos02;
  
  if( !s )
  {
    return 0;  
  }
  
  copy = ecstr_copy (s);
  
  /* source position */
  pos01 = copy;
  pos02 = 0;
  /* trim from begin */
  while(*pos01)
  {
    if (*pos01 != l) break;
    
    pos01++;
  }
  pos02 = copy;
  /* copy rest */
  while(*pos01)
  {
    *pos02 = *pos01;
    
    pos01++;
    pos02++;
  }
  /* set here 0 not to run in undefined situation */
  *pos02 = 0;
  /* trim from the end */
  while( pos02 != copy )
  {
    /* decrease */
    pos02--;
    /* check if readable */
    if (*pos02 != r) break;
    /* set to zero */
    *pos02 = 0;    
  }  
  
  return copy;
}

/*------------------------------------------------------------------------*/

EcString ecstr_trimKeepDefault( const EcString s )
{
  EcString copy = ecstr_copy( s );

  return copy;
}

/*------------------------------------------------------------------------*/

EcString ecstr_trimEndLine( const EcString s )
{
  uint_t size = strlen(s);
  
  const char* pos; 
  
  for(pos = s + size; (*pos == '\n') && (*pos == '\r'); pos--)
  {
  }
  
  return ecstr_part(s, pos - s - 1);
}

//---------------------------------------------------------------------------

EcString ecstr_unwrapl (const EcString source, char cl, char cr, EcString* pleft, EcString* pright)
{
  // look for the left char
  EcString sl;
  EcString sr;
  
  sl = strchr (source, cl);
  if (sl == NULL)
  {
    if (pleft)
    {
      *pleft = NULL;
    }
    
    if (pright)
    {
      *pright = NULL;
    }
    
    // no first left char found
    return ecstr_copy (source);
  }
  
  sr = strchr (sl + 1, cr);
  if (sr == NULL)
  {
    if (pleft)
    {
      *pleft = NULL;
    }
    
    if (pright)
    {
      *pright = NULL;
    }
    
    // no first left char found
    return ecstr_copy (source);
  }

  if (pleft)
  {
    EcString left;
    
    // just calculate the diff from sl to the begining
    unsigned long diff = sl - source;

    if (diff > 0)
    {
      left = ecstr_part (source, diff);
    }
    else
    {
      left = NULL;
    }
    
    *pleft = left;
  }
  
  if (pright)
  {
    EcString right;
    
    // find the end of the string
    const char* se;
    
    for (se = sr; *se; se++);
    
    {
      unsigned long diff = se - sr - 1;
      
      if (diff > 0)
      {
        right = ecstr_part (sr + 1, diff);
      }
      else
      {
        right = NULL;
      }
    }
    
    *pright = right;
  }
  
  // compose the return
  return ecstr_part (sl + 1, sr - sl - 1);
}

//---------------------------------------------------------------------------

EcString ecstr_wrappedl (const EcString source, char c)
{
  return ecstr_unwrapl (source, c, c, NULL, NULL);
}

//---------------------------------------------------------------------------

EcString ecstr_lpad (const EcString source, char c, uint_t len)
{
  uint_t slen = ecstr_len (source);
  uint_t dlen = len - slen;
  
  if (dlen > 0)
  {
    EcString h = ecstr_filled (c, dlen);
    
    return ecstr_replaceTO (&h, ecstr_cat2 (h, source));
  }
  else
  {
    return ecstr_copy (source);
  }
}

/*------------------------------------------------------------------------*/

void ecstr_replaceAllChars( EcString self, char find, char replace )
{
  char* pos = self;
  while (*pos)
  {
    if (*pos == find) 
    {
      *pos = replace;
    }
    pos++;
  }
}

//-------------------------------------------------------------------------------

EcString ecstr_removeAllChars (const EcString source, char find)
{
  EcString r;
  int len = ecstr_len (source);
  
  const char* s = source;
  char* d;

  // allocate memory
  r = ENTC_MALLOC (len + 1);
  d = r;
  
  // copy content without the find char
  while (*s)
  {
    if (*s != find)
    {
      *d = *s;
      d++;
    }
    
    s++;
  }
  
  *d = '\0';
  
  return r;
}

//-------------------------------------------------------------------------------

EcString ecstr_replaceS (const EcString source, const EcString find, const EcString replace)
{
  EcStream s;
  const char* fpos;
  const char* lpos;
  
  if (source == NULL)
  {
    return NULL;
  }
  
  if (find == NULL)
  {
    return ecstr_copy (source);
  }
  
  s = ecstream_create ();
  
  lpos = source;
  
  for (fpos = strstr (lpos, find); fpos; fpos = strstr (lpos, find))
  {
    ecstream_append_buf (s, lpos, fpos - lpos);

    if (replace)
    {
      ecstream_append_str (s, replace);
    }
    
    lpos = fpos + strlen (find);
  }

  ecstream_append_str (s, lpos);
  
  {
    EcBuffer b = ecstream_tobuf (&s);
    
    return ecbuf_str (&b);
  }
}

/*------------------------------------------------------------------------*/

void ecstr_toLower(EcString self)
{
  char* pos = self;
  while (*pos)
  {
    *pos = tolower(*pos);
    pos++;
  }  
}

/*------------------------------------------------------------------------*/

void ecstr_toUpper(EcString self)
{
  char* pos = self;
  while (*pos)
  {
    *pos = toupper(*pos);
    pos++;
  }
}

/*------------------------------------------------------------------------*/

const EcString ecstr_pos (const EcString source, char c)
{
  return strchr (source, c);
}

//-------------------------------------------------------------------------------

const EcString ecstr_spos (const EcString source, char c, EcString* part)
{
  const char* pos = strchr (source, c);
  if (pos == NULL) 
  {
    // easy just return the source
    ecstr_replace (part, source);
    
    return NULL;
  }
  else
  {
    ecstr_replaceTO (part, ecstr_part(source, pos - source));

    return pos + 1;
  }  
}

//-------------------------------------------------------------------------------

const EcString ecstr_npos (const EcString source, char c, uint_t max)
{
  const EcString pos = source;
  
  uint_t i = 0;
  
  for(; i < max; ++i, ++pos)
  {
    if(*pos == c) return pos;
  }
  
  /* not found anything */
  return 0;  
}

/*------------------------------------------------------------------------*/

EcString ecstr_extractf (const EcString source, char c)
{
  EcString part = ecstr_init ();
  
  ecstr_spos (source, c, &part);
  
  return part;
}

//----------------------------------------------------------------------------------------

EcString ecstr_shrink (const EcString source, char from, char to)
{
  const char* pos1;
  const char* pos2;

  pos1 = strchr(source, from);
  if (isNotAssigned (pos1))
  {
    pos1 = source;
  }
  else
  {
    pos1++;
  }

  pos2 = strrchr(source, to);
  if (isNotAssigned (pos1))
  {
    pos2 = source + strlen(source);
  }
  
  return ecstr_part(pos1, pos2 - pos1);
}

//----------------------------------------------------------------------------------------

int ecstr_split (const EcString source, EcString* s1, EcString* s2, char c)
{
  const char * pos = strchr ( source, c );
  if (pos != NULL)
  {
    EcString h = ecstr_part(source, pos - source);
  
    ecstr_replaceTO (s1, h);
    ecstr_replace (s2, pos + 1);
    
    return TRUE;
  }
  
  return FALSE;
}

/*------------------------------------------------------------------------*/

int ecstr_empty(const EcString self)
{
  if( ecstr_valid(self) )
  {
    if( *self != 0 )
    {
      return FALSE;
    }
  }
  
  return TRUE;
}

/*------------------------------------------------------------------------*/

EcString ecstr_extractParameter(char pn, int argc, char *argv[])
{
  int i;
  
  for (i = 0; i < argc; i++)
  {
    // all parameters have this form : [pn]=[value]
    // search for the pattern
    const char* arg = argv[i];
    
    if( strlen(arg) > 3 )
    {
      if ((arg[0] == pn) && (arg[1] == '='))
      {
        return ecstr_copy(arg + 2);
      }
      
    }
  }
  
  return 0;
}

/*------------------------------------------------------------------------*/

EcString ecstr_toVersion( uint_t version )
{
  uint_t size;
  EcBuffer buffer01 = ecbuf_create (11);
  EcBuffer buffer02;
  
  unsigned char* pos01;
  unsigned char* pos02;
  
  ecbuf_format (buffer01, 10, "%05lu", version);
  
  size = strlen((char*)buffer01->buffer);
  
  buffer02 = ecbuf_create (size + 3);
  
  pos01 = buffer01->buffer;
  pos02 = buffer02->buffer;

  while (*pos01) 
  {
    *pos02 = *pos01;
    pos01++;
    pos02++;
    
    {
      uint_t diff = (pos01 - buffer01->buffer);
    
      if ((diff == (size - 2)) || (diff == (size - 4)) ) {
        *pos02 = '.';
        pos02++;
      }
    }
  }
  
  *pos02 = 0;
  
  ecbuf_destroy (&buffer01);
    
  return ecbuf_str (&buffer02);
}

//-----------------------------------------------------------------------------

wchar_t* ecstr_utf8ToWide (const EcString source)
{

#ifdef _WIN32

  int size = MultiByteToWideChar (CP_ACP, MB_COMPOSITE, source, -1, NULL, 0);

  wchar_t* wdest = malloc(sizeof(wchar_t) * size);

  int res = MultiByteToWideChar (CP_ACP, MB_COMPOSITE, source, -1, wdest, size);

  return wdest;

#else

  return NULL;

#endif
}

//-----------------------------------------------------------------------------
