#include "entc_stream.h"

// c includes
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <memory.h>
#include <limits.h>
#include <stdlib.h>

//-----------------------------------------------------------------------------

struct EntcStream_s
{
  number_t size;
  
  char* buffer;
  
  char* pos;
  
};

//-----------------------------------------------------------------------------

number_t entc_stream_size (EntcStream self)
{
  return self->pos - self->buffer;
}

//-----------------------------------------------------------------------------

void entc_stream_allocate (EntcStream self, unsigned long amount)
{
  // safe how much we have used from the buffer
  unsigned long usedBytes = entc_stream_size (self);
  
  // use realloc to minimalize coping the buffer
  self->size += amount;
  self->buffer = realloc(self->buffer, self->size + 1);
  
  // reset the position
  self->pos = self->buffer + usedBytes;
}

//-----------------------------------------------------------------------------

void entc_stream_reserve (EntcStream self, unsigned long amount)
{
  unsigned long diffBytes = entc_stream_size (self) + amount + 1;
  
  if (diffBytes > self->size)
  {
    if (amount > self->size)
    {
      entc_stream_allocate (self, amount);
    }
    else
    {
      entc_stream_allocate (self, self->size + self->size);
    }
  }
}

//-----------------------------------------------------------------------------

EntcStream entc_stream_new ()
{
  EntcStream self = ENTC_NEW(struct EntcStream_s);
  
  self->size = 0;
  self->buffer = 0;
  self->pos = self->buffer;
  
  // initial alloc
  entc_stream_allocate (self, 100);
  
  // clean
  entc_stream_clr (self);
  
  return self;
}

//-----------------------------------------------------------------------------

void entc_stream_del (EntcStream* pself)
{
  EntcStream self = *pself;
  
  if (self)
  {
    free (self->buffer);
    
    ENTC_DEL (pself, struct EntcStream_s);
  }  
}

//-----------------------------------------------------------------------------

EntcString entc_stream_to_str (EntcStream* pself)
{
  EntcStream self = *pself;  
  EntcString ret = self->buffer;
  
  // set terminator
  *(self->pos) = 0;
  
  ENTC_DEL(pself, struct EntcStream_s);
  
  return ret;
}

//-----------------------------------------------------------------------------

number_t entc_stream_to_n (EntcStream self)
{
  // prepare and add termination
  *(self->pos) = '\0';
  
  number_t ret = strtol (self->buffer, NULL, 10);
    
  entc_stream_clr (self);
  
  return ret;
}

//-----------------------------------------------------------------------------

EntcString entc_stream_to_s (EntcStream self)
{
  EntcString ret = entc_str_sub (self->buffer, self->pos - self->buffer);
  
  entc_stream_clr (self);
  
  return ret;
}

//-----------------------------------------------------------------------------

void entc_stream_clr (EntcStream self)
{
  self->pos = self->buffer;
}

//-----------------------------------------------------------------------------

const char* entc_stream_get (EntcStream self)
{
  // set terminator
  *(self->pos) = 0;
  
  return self->buffer;
}

//-----------------------------------------------------------------------------

const char* entc_stream_data (EntcStream self)
{
  return self->buffer;
}

//-----------------------------------------------------------------------------

void entc_stream_append_str (EntcStream self, const char* s)
{
  if (s)
  {
    // need to find the length
    entc_stream_append_buf (self, s, strlen(s));
  }
}

//-----------------------------------------------------------------------------

void entc_stream_append_buf (EntcStream self, const char* buffer, unsigned long size)
{
  if (size > 0)
  {
    entc_stream_reserve (self, size);
    
    memcpy (self->pos, buffer, size);
    self->pos += size;
  }
}

//-----------------------------------------------------------------------------

void entc_stream_append_fmt (EntcStream self, const char* format, ...)
{
  // variables
  va_list valist;
  va_start (valist, format);
  
  #ifdef _MSC_VER
  
  {
    int len = _vscprintf (format, valist) + 1;
    
    entc_stream_reserve (self, len);
    
    len = vsprintf_s (self->pos, len, format, valist);
    
    self->pos += len;
  }
  
  #elif _GCC
  
  {
    char* strp;
    
    int bytesWritten = vasprintf (&strp, format, valist);
    if ((bytesWritten > 0) && strp)
    {
      entc_stream_append_buf (self, strp, bytesWritten);
      free(strp);
    }
  }
  
  #elif __BORLANDC__
  
  {
    int len = 1024;
    
    entc_stream_reserve (self, len);
    
    len = vsnprintf (self->pos, len, format, valist);
    
    self->pos += len;
  }
  
  #endif
  
  va_end(valist);
}

//-----------------------------------------------------------------------------

void entc_stream_append_c (EntcStream self, char c)
{
  entc_stream_reserve (self, 1);
  
  *(self->pos) = c;
  self->pos++;
}

//-----------------------------------------------------------------------------

void entc_stream_append_n (EntcStream self, number_t val)
{
  entc_stream_reserve (self, 26);  // for very long intergers
  
#ifdef _MSC_VER
  
  self->pos += _snprintf_s (self->pos, 24, _TRUNCATE, "%lu", val);
  
#else
  
  self->pos += snprintf(self->pos, 24, "%lu", val);
  
#endif
}

//-----------------------------------------------------------------------------

void entc_stream_append_f (EntcStream self, double val)
{
  entc_stream_reserve (self, 26);  // for very long intergers
  
#ifdef _MSC_VER
  
  self->pos += _snprintf_s (self->pos, 24, _TRUNCATE, "%f", val);
  
#else
  
  self->pos += snprintf(self->pos, 24, "%f", val);
  
#endif
}

//-----------------------------------------------------------------------------

void entc_stream_append_stream (EntcStream self, EntcStream stream)
{
  unsigned long usedBytes = stream->pos - stream->buffer;
  
  entc_stream_reserve (self, usedBytes);
  
  memcpy (self->pos, stream->buffer, usedBytes);
  self->pos += usedBytes;
}

//-----------------------------------------------------------------------------
