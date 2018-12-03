#include "ecstream.h"

// entc includes
#include "system/macros.h"
#include "system/ecfile.h"

// c includes
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <memory.h>
#include <limits.h>

//-----------------------------------------------------------------------------

struct EcStream_s
{
  
  unsigned long size;
  
  char* buffer;
  
  char* pos;
  
};

//-----------------------------------------------------------------------------

unsigned long ecstream_size (EcStream self)
{
  return self->pos - self->buffer;
}

//-----------------------------------------------------------------------------

void ecstream_allocate (EcStream self, unsigned long amount)
{
  // safe how much we have used from the buffer
  unsigned long usedBytes = ecstream_size (self);
  
  // use realloc to minimalize coping the buffer
  self->size += amount;
  self->buffer = realloc(self->buffer, self->size);
  
  // reset the position
  self->pos = self->buffer + usedBytes;
}

//-----------------------------------------------------------------------------

void ecstream_reserve (EcStream self, unsigned long amount)
{
  unsigned long diffBytes = ecstream_size (self) + amount + 1;
  
  if (diffBytes > self->size)
  {
    if (amount > self->size)
    {
      ecstream_allocate (self, amount);
    }
    else
    {
      ecstream_allocate (self, self->size + self->size);
    }
  }
}

//-----------------------------------------------------------------------------

EcStream ecstream_create ()
{
  EcStream self = ENTC_NEW(struct EcStream_s);
  
  self->size = 0;
  self->buffer = 0;
  self->pos = self->buffer;
  
  // initial alloc
  ecstream_allocate (self, 100);
  
  // clean
  ecstream_clear (self);
  
  return self;
}

//-----------------------------------------------------------------------------

void ecstream_destroy (EcStream* pself)
{
  EcStream self = *pself;
  
  free (self->buffer);
  
  ENTC_DEL (pself, struct EcStream_s);
}

//-----------------------------------------------------------------------------

EcBuffer ecstream_tobuf (EcStream* pself)
{
  EcStream self = *pself;
  EcBuffer ret = ENTC_NEW (EcBuffer_s);
  
  // set terminator
  *(self->pos) = 0;

  ret->size = ecstream_size (self);
  ret->buffer = (unsigned char*)self->buffer;
  
  ENTC_DEL(pself, struct EcStream_s);
  
  return ret;
}

//-----------------------------------------------------------------------------

EcString ecstream_tostr (EcStream* pself)
{
  EcStream self = *pself;  
  EcString ret = self->buffer;
  
  // set terminator
  *(self->pos) = 0;
  
  ENTC_DEL(pself, struct EcStream_s);
   
  return ret;
}

//-----------------------------------------------------------------------------

void ecstream_clear (EcStream self)
{
  self->pos = self->buffer;
}

//-----------------------------------------------------------------------------

const char* ecstream_get (EcStream self)
{
  // set terminator
  *(self->pos) = 0;
  
  return self->buffer;
}

//-----------------------------------------------------------------------------

void ecstream_append_str (EcStream self, const char* s)
{
  if (s)
  {
    // need to find the length
    ecstream_append_buf (self, s, strlen(s));
  }
}

//-----------------------------------------------------------------------------

void ecstream_append_buf (EcStream self, const char* buffer, unsigned long size)
{
  if (size > 0)
  {
    ecstream_reserve (self, size);
    
    memcpy (self->pos, buffer, size);
    self->pos += size;
  }
}

//-----------------------------------------------------------------------------

void ecstream_append_ecbuf (EcStream self, const EcBuffer buf)
{
  if (buf->size > 0)
  {
    ecstream_reserve (self, buf->size);
    
    memcpy (self->pos, buf->buffer, buf->size);
    self->pos += buf->size;
  }  
}

//-----------------------------------------------------------------------------

void ecstream_append_fmt (EcStream self, const char* format, ...)
{
  // variables
  va_list valist;
  va_start (valist, format);
  
#ifdef _MSC_VER
  
  {
    int len = _vscprintf (format, valist) + 1;
    
    ecstream_reserve (self, len);
    
    len = vsprintf_s (self->pos, len, format, valist);
    
    self->pos += len;
  }
  
#elif _GCC
  
  {
    char* strp;
    
    int bytesWritten = vasprintf (&strp, format, valist);
    if ((bytesWritten > 0) && strp)
    {
      ecstream_append_buf (self, strp, bytesWritten);
      free(strp);
    }
  }
  
#elif __BORLANDC__
  
  {
    int len = 1024;
    
    ecstream_reserve (self, len);
    
    len = vsnprintf (self->pos, len, format, valist);
    
    self->pos += len;
  }
  
#endif
  
  va_end(valist);
}

//-----------------------------------------------------------------------------

void ecstream_append_c (EcStream self, char c)
{
  ecstream_reserve (self, 1);
  
  *(self->pos) = c;
  self->pos++;
}

//-----------------------------------------------------------------------------

void ecstream_append_u (EcStream self, unsigned long val)
{
  ecstream_reserve (self, 26);  // for very long intergers
  
#ifdef _MSC_VER

  self->pos += _snprintf_s (self->pos, 24, _TRUNCATE, "%lu", val);

#else

  self->pos += snprintf(self->pos, 24, "%lu", val);

#endif
}

//-----------------------------------------------------------------------------

void ecstream_append_u64 (EcStream self, uint64_t val)
{
  ecstream_reserve (self, 26);  // for very long intergers
  
#ifdef _MSC_VER

  self->pos += _snprintf_s (self->pos, 24, _TRUNCATE, "%lu", val);

#else

  self->pos += snprintf(self->pos, 24, "%lu", val);

#endif
}

//-----------------------------------------------------------------------------

void ecstream_append_i (EcStream self, long val)
{
  ecstream_reserve (self, 26);  // for very long intergers
 
#ifdef _MSC_VER

  self->pos += _snprintf_s (self->pos, 24, _TRUNCATE, "%li", val);

#else
 
  self->pos += snprintf(self->pos, 24, "%li", val);

#endif
}

//-----------------------------------------------------------------------------

void ecstream_append_i64 (EcStream self, int64_t val)
{
  ecstream_reserve (self, 26);  // for very long intergers
 
#ifdef _MSC_VER

  self->pos += _snprintf_s (self->pos, 24, _TRUNCATE, "%li", val);

#else
 
  self->pos += snprintf(self->pos, 24, "%li", val);

#endif
}

//-----------------------------------------------------------------------------

void ecstream_append_time (EcStream self, const time_t* t)
{
  ecstream_reserve (self, 32);  // for very long intergers

#ifdef _WIN32

  self->pos += _snprintf_s (self->pos, 30, _TRUNCATE, "%lu", (unsigned long)*t);
  
#else

  self->pos += snprintf(self->pos, 30, "%lu", (unsigned long)*t);

#endif
}

//-----------------------------------------------------------------------------

void ecstream_append_stream (EcStream self, EcStream stream)
{
  unsigned long usedBytes = stream->pos - stream->buffer;
  
  ecstream_reserve (self, usedBytes);
  
  memcpy (self->pos, stream->buffer, usedBytes);
  self->pos += usedBytes;
}

//=======================================================================================

struct EcDevStream_s
{
  EcBuffer buffer;
  
  unsigned char* pos;
  
  stream_callback_fct fct;
  
  void* ptr;
  
};

//---------------------------------------------------------------------------------------

EcDevStream ecdevstream_new (uint_t size, stream_callback_fct fct, void* ptr)
{
  EcDevStream self = ENTC_NEW(struct EcDevStream_s);
  
  self->buffer = ecbuf_create (size);
  self->pos = self->buffer->buffer;
  
  self->fct = fct;
  self->ptr = ptr;
  
  return self;
}

//---------------------------------------------------------------------------------------

void ecdevstream_delete (EcDevStream* pself)
{
  EcDevStream self = *pself;
  
  ecdevstream_flush(self);
  
  ecbuf_destroy (&(self->buffer));
  
  ENTC_DEL(pself, struct EcDevStream_s);
}

//---------------------------------------------------------------------------------------

void ecdevstream_flush (EcDevStream self)
{
  if (self->pos != self->buffer->buffer)
  {
    self->fct(self->ptr, self->buffer->buffer, self->pos - self->buffer->buffer);
    
    self->pos = self->buffer->buffer;
  }
}

//---------------------------------------------------------------------------------------

void ecdevstream_append (EcDevStream self, void* source, uint_t size)
{
  uint_t rsize = size;
  const char* pos = source;
  
  while (TRUE)
  {
    uint_t filled = self->pos - self->buffer->buffer;
    uint_t diff = self->buffer->size - filled;
    
    if (diff < rsize)
    {
      memcpy(self->pos, pos, diff);
      self->fct(self->ptr, self->buffer->buffer, self->buffer->size);
      
      rsize = rsize - diff;
      pos = pos + diff;
      
      self->pos = self->buffer->buffer;
    }
    else
    {
      memcpy(self->pos, pos, rsize);
      self->pos = self->pos + rsize;
      break;
    }
  }
}

//---------------------------------------------------------------------------------------

void ecdevstream_appends (EcDevStream self, const EcString source)
{
  // check the value
  if( ecstr_empty(source) )
  {
    return;
  }
  
  ecdevstream_append (self, (void*)source, strlen(source));
}

//---------------------------------------------------------------------------------------

void ecdevstream_appendc (EcDevStream self, char c)
{
  ecdevstream_append (self, &c, 1);
}

//---------------------------------------------------------------------------------------

void ecdevstream_appendu (EcDevStream self, uint_t value)
{
  EcString h = ecstr_long(value);
  /* add */
  ecdevstream_appends ( self, h );
  
  ecstr_delete(&h);
}

//---------------------------------------------------------------------------------------

void ecdevstream_appendfile (EcDevStream self, const EcString filename)
{
  EcFileHandle fh = ecfh_open(filename, O_RDONLY);
  if (fh)
  {
    EcBuffer buffer = ecbuf_create (1024);
    
    uint_t res = ecfh_readBuffer(fh, buffer);
    while (res > 0)
    {
      ecdevstream_append (self, buffer->buffer, res);
      res = ecfh_readBuffer(fh, buffer);
    }
    
    ecfh_close(&fh);
    
    ecbuf_destroy (&buffer);
  }
  else
  {
    ecdevstream_appends (self, "fle read error");
  }
}

//---------------------------------------------------------------------------------------
