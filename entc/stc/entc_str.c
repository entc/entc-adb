/*
 Copyright (c) 2019 Alexander Kalkhof
 
 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:
 
 The above copyright notice and this permission notice shall be included in all
 copies or substantial portions of the Software.
 
 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 SOFTWARE.
 */

#include "entc_str.h"

#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <ctype.h>
#include <stdio.h>

//-----------------------------------------------------------------------------

EntcString entc_str_cp (const EntcString source)
{
  if (source == NULL)
  {
    return NULL;
  }  

#ifdef _WIN32
  return _strdup (source);  
#else
  return strdup (source);
#endif    
}

//-----------------------------------------------------------------------------

EntcString entc_str_sub (const EntcString source, number_t len)
{
  char* ret;
  
  // check if we have at least value 
  if (source == NULL)
  {
    return NULL;
  }  
  
  ret = (char*)ENTC_ALLOC( (2 + len) * sizeof(char) );
  
  /* copy the part */
#ifdef _WIN32
  memcpy(ret, source, len * sizeof(char));
#else
  strncpy(ret, source, len);
#endif
  /* set the termination */
  ret[len] = 0;
  
  return ret; 
}

//-----------------------------------------------------------------------------

void entc_str_del (EntcString* p_self)
{
  EntcString self = *p_self;
    
  if(self)
  {
    memset (self, 0x00, strlen(self));
    free (self);
  }
  
  *p_self = NULL;
}

//-----------------------------------------------------------------------------

EntcString entc_str_uuid (void)
{
  EntcString self = ENTC_ALLOC(38);
  
  sprintf(self, "%04X%04X-%04X-%04X-%04X-%04X%04X%04X", 
          rand() & 0xffff, rand() & 0xffff,                          // Generates a 64-bit Hex number
          rand() & 0xffff,                                           // Generates a 32-bit Hex number
          ((rand() & 0x0fff) | 0x4000),                              // Generates a 32-bit Hex number of the form 4xxx (4 indicates the UUID version)
          rand() % 0x3fff + 0x8000,                                  // Generates a 32-bit Hex number in the range [0x8000, 0xbfff]
          rand() & 0xffff, rand() & 0xffff, rand() & 0xffff);        // Generates a 96-bit Hex number
  
  return self;
}

//-----------------------------------------------------------------------------

EntcString entc_str_catenate_2 (const EntcString s1, const EntcString s2)
{
  if( !s1 )
  {
    return entc_str_cp (s2);  
  }
  
  if( !s2 )
  {
    return entc_str_cp (s1);  
  }
  
  {
    /* variables */
    int s1_len = strlen(s1);
    int s2_len = strlen(s2);
    
    char* ret;
    char* pos;
    
    ret = (char*)ENTC_ALLOC( (s1_len + s2_len + 10) );
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

//-----------------------------------------------------------------------------

EntcString entc_str_catenate_3 (const EntcString s1, const EntcString s2, const EntcString s3)
{
  number_t s1_len = strlen(s1);
  number_t s2_len = strlen(s2);
  number_t s3_len = strlen(s3);
  
  char* ret = (char*)ENTC_ALLOC( (s1_len + s2_len + s3_len + 1) * sizeof(char) );
  
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

//-----------------------------------------------------------------------------

EntcString entc_str_catenate_c (const EntcString s1, char c, const EntcString s2)
{
  // variables
  number_t s1_len;
  number_t s2_len;
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
  
  ret = (char*)ENTC_ALLOC( (s1_len + s2_len + 2) );
  
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

//-----------------------------------------------------------------------------

void entc_str_replace_cp (EntcString* p_self, const EntcString source)
{
  // create a new copy
  EntcString self = entc_str_cp (source);

  // free the old string  
  entc_str_del (p_self);
  
  // transfer ownership
  *p_self = self;
}

//-----------------------------------------------------------------------------

void entc_str_replace_mv (EntcString* p_self, EntcString* p_source)
{
  // free the old string  
  entc_str_del (p_self);

  // transfer ownership
  *p_self = *p_source;

  // release ownership
  *p_source = NULL;
}

//-----------------------------------------------------------------------------

void entc_str_fill (EntcString self, number_t len, const EntcString source)
{
  // assume that the source will fit into the self object
  strncpy (self, source, len);
  
  self[len] = '\0';
}

//-----------------------------------------------------------------------------

void entc_str_to_upper (EntcString self)
{
  char* pos = self;
  
  while (*pos)
  {
    *pos = toupper (*pos);
    pos++;
  }
}

//-----------------------------------------------------------------------------

void entc_str_to_lower (EntcString self)
{
  char* pos = self;
  
  while (*pos)
  {
    *pos = tolower (*pos);
    pos++;
  }
}

//-----------------------------------------------------------------------------

