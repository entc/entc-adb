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

#include <string.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

/*------------------------------------------------------------------------*/

EcString ecstr_init()
{
  return 0;  
}

/*------------------------------------------------------------------------*/

EcBuffer ecstr_buffer( uint_t size )
{
  return ecstr_bufferFilled (size, 0x00);
}

/*------------------------------------------------------------------------*/

EcBuffer ecstr_bufferFilled (uint_t size, char fillupwith)
{
  EcBuffer self = ENTC_NEW(struct EcBuffer_s);
  
  self->buffer =  (unsigned char*)ENTC_MALLOC( (1 + size) * sizeof(unsigned char) );
  self->size = size;
  
  memset(self->buffer, fillupwith, size);
  
  ecstr_bufferSetTerm (self, size);
  
  return self;  
}

/*------------------------------------------------------------------------*/

void ecstr_bufferSetTerm (EcBuffer self, uint_t size)
{
  if (size <= self->size)
  {
    // set c-string termination
    self->buffer[size] = 0;
  }
}

/*------------------------------------------------------------------------*/

void ecstr_bufferFillWith (EcBuffer self, uint_t size, char fillupwith)
{
  uint_t fillup = size < self->size ? size : self->size;
  
  memset(self->buffer, fillupwith, fillup);

  ecstr_bufferSetTerm (self, fillup);  
}

/*------------------------------------------------------------------------*/

void ecstr_resize( EcBuffer self, uint_t size )
{
  self->buffer = (unsigned char*) realloc (self->buffer, size);
  self->size = size;
}

/*------------------------------------------------------------------------*/

const EcString ecstr_get( const EcBuffer self )
{
  return (const EcString)self->buffer;
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

EcString ecstr_part( const EcString source, uint_t length )
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

/*------------------------------------------------------------------------*/

EcString ecstr_long( uint_t value )
{
  /* create buffer with size 12 */
  EcBuffer buffer = ecstr_buffer(12);
  /* transform unsigned value into string */  
  ecstr_format(buffer, 11, "%lu", value );
  /* transform buffer into string */
  return ecstr_trans(&buffer);
}

/*------------------------------------------------------------------------*/

EcString ecstr_float (double value, uint_t n)
{
  EcString s = ecstr_init ();
  if( n > 0 )
  {
	  EcBuffer buffer01 = ecstr_buffer(7);
	  EcBuffer buffer02 = ecstr_buffer(31);
    
    ecstr_format(buffer01, 6, "%%.%uf", n);
    /* transform to integer */
    ecstr_format(buffer02, 30, ecstr_get(buffer01), value );
    /* copy */
	  ecstr_replaceTrans(&s, &buffer02);
	  ecstr_release(&buffer01);
  }
  else
  {
	  EcBuffer buffer02 = ecstr_buffer(31);
    /* transform to integer */
    ecstr_format(buffer02, 30, "%f", value );
    /* copy */
	  ecstr_replaceTrans(&s, &buffer02);
  }
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

void ecstr_release( EcBuffer* ptr )
{
  EcBuffer self = *ptr;
  /* clear the whole string */
  memset(self->buffer, 0x00, self->size);
  /* clear memory */  
  ENTC_FREE(self->buffer);
  
  ENTC_DEL(ptr, struct EcBuffer_s);
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

/*------------------------------------------------------------------------*/

int ecstr_equaln( const EcString s1, const EcString s2, uint_t size )
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

int ecstr_leading( const EcString s, const EcString leading)
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

void ecstr_random( EcBuffer buffer, uint_t size )
{
  uint_t i;
  
  for(i = 0; (i < size) && (i < buffer->size); i++)
  {
    (buffer->buffer)[i] = (rand() % 26) + 97;
  }
  (buffer->buffer)[i] = 0;  
}

/*------------------------------------------------------------------------*/

void ecstr_format( EcBuffer buffer, uint_t size, const char* format, ...)
{
  va_list ptr;  
  va_start(ptr, format);
#ifdef _WIN32
  vsnprintf_s( (char*)buffer->buffer, buffer->size, size, format, ptr );
#elif __DOS__
  vsprintf((char*)buffer->buffer, format, ptr);
#else
  vsnprintf((char*)buffer->buffer, buffer->size, format, ptr );
#endif
  va_end(ptr);  
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
  }
  else
  {
    ecstr_replaceTO (part, ecstr_part(source, pos - source));
  }  
  return pos;
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
    ecstr_replaceTO (s1, ecstr_part(source, pos - source));
    ecstr_replace (s2, pos + 1);
    
    return TRUE;
  }
  return FALSE;
}

/*------------------------------------------------------------------------*/

void ecstr_tokenizer_clear(EcList list)
{
  EcListNode node;
  for(node = eclist_first(list); node != eclist_end(list); node = eclist_next(node))
  {
    EcString h = eclist_data(node);
    
    ecstr_delete( &h );
  }
  eclist_clear( list );
}

/*------------------------------------------------------------------------*/

EcString ecstr_tokenizer_get (EcList list, EcListNode node)
{
  if (node != eclist_end(list))
  {
	return eclist_data (node);
  }
  return ecstr_init();
}

/*------------------------------------------------------------------------*/

void ecstr_tokenizer(EcList list, const EcString source, char delimeter)
{
  const char* posR = source;
  const char* posL = source;

  if (ecstr_empty(source)) {
    return;
  }
  
  ecstr_tokenizer_clear(list);
  
  while (*posR)
  {
    if (*posR == delimeter)
    {
      // calculate the length of the last segment
      uint_t diff = posR - posL;
      if (diff > 0)
      {
        // allocate memory for the segment
        char* buffer = (char*)ENTC_MALLOC(diff + 2);
        memcpy(buffer, posL, diff);
        // set termination
        buffer[diff] = 0;
        // add to list      
        eclist_append(list, buffer);
      }
      posL = posR + 1;
    }
    posR++;
  }
  
 
  {
    // calculate the length of the last segment
    uint_t diff = posR - posL;
    if (diff > 0)
    {
      // allocate memory for the segment
      char* buffer = (char*)ENTC_MALLOC(diff + 1);      
      memcpy(buffer, posL, diff);
      // set termination
      buffer[diff] = 0;
      // add to list      
      eclist_append(list, buffer);      
    }
  }
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

EcString ecstr_trans(EcBuffer* ptr)
{
  EcBuffer self = *ptr;
  // transform buffer content into string
  EcString ret = (char*)self->buffer;
  // delete buffer struct
  ENTC_DEL(ptr, struct EcBuffer_s);
  // return string
  return ret;
}

/*------------------------------------------------------------------------*/

void ecstr_replaceTrans(EcString* string, EcBuffer* buffer)
{
  ecstr_delete(string);
  *string = ecstr_trans(buffer);
}

/*------------------------------------------------------------------------*/

EcString ecstr_localtime(time_t time)
{
  static const char* month_matrix[12] = {"Jan","Feb","Mar","Apr","May","Jun","Jul","Aug","Sep","Oct","Nov","Dec"};
  /* variables */
  EcBuffer buffer = ecstr_buffer(30);
#ifdef _WIN32
  struct tm l01;
  localtime_s(&l01, &time);
  ecstr_format(buffer, 28, "%02u-%s-%04u %02u:%02u:%02u", l01.tm_mday, month_matrix[l01.tm_mon], (l01.tm_year + 1900), l01.tm_hour, l01.tm_min, l01.tm_sec);
#else
  struct tm* l01 = localtime( &time );
  ecstr_format(buffer, 28, "%02u-%s-%04u %02u:%02u:%02u", l01->tm_mday, month_matrix[l01->tm_mon], (l01->tm_year + 1900), l01->tm_hour, l01->tm_min, l01->tm_sec);
#endif
  return ecstr_trans(&buffer);
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
  EcBuffer buffer01 = ecstr_buffer(11);
  EcBuffer buffer02;
  
  unsigned char* pos01;
  unsigned char* pos02;
  
  ecstr_format(buffer01, 10, "%05lu", version);
  
  size = strlen((char*)buffer01->buffer);
  
  buffer02 = ecstr_buffer(size + 3);
  
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
  
  ecstr_release(&buffer01);
    
  return ecstr_trans(&buffer02);
}

/*------------------------------------------------------------------------*/
