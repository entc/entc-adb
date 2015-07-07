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

#include "ecstream.h"

#include <system/ecfile.h>

#include <string.h>
#include <fcntl.h>

struct EcStream_s
{
  EcBuffer buffer;
  
  unsigned char* pos;
};

/*------------------------------------------------------------------------*/

EcStream ecstream_new(void)
{
  EcStream self = ENTC_NEW(struct EcStream_s);

  self->buffer = ecbuf_create (1000);
  self->pos = self->buffer->buffer;
  
  return self;
}

/*------------------------------------------------------------------------*/

void ecstream_flush(EcStream self)
{
  
}

/*------------------------------------------------------------------------*/

void ecstream_delete(EcStream* pself)
{
  EcStream self = *pself;
  
  ecstream_flush(self);
  
  ecbuf_destroy (&(self->buffer));
  
  ENTC_DEL(pself, struct EcStream_s);
}

/*------------------------------------------------------------------------*/

void ecstream_clear( EcStream self )
{
  // reset the position pointer
  self->pos = self->buffer->buffer;
  *(self->pos) = 0;
}

/*------------------------------------------------------------------------*/

EcBuffer ecstream_trans (EcStream* pself)
{
  EcStream self = *pself;
  
  EcBuffer ret = self->buffer;
  
  ecstream_flush(self);
  ret->size = ecstream_size (self);
  
  ENTC_DEL(pself, struct EcStream_s);
  
  return ret;
}

/*------------------------------------------------------------------------*/

const EcString ecstream_buffer( EcStream self )
{
  return (EcString)self->buffer->buffer;
}

/*------------------------------------------------------------------------*/

void _ecstream_check (EcStream self, uint_t size)
{
  uint_t filled = 0;
  // already filled bytes
  filled = self->pos - self->buffer->buffer;
  // remaining bytes free + zero termination
  if(size > (self->buffer->size - filled - 1))
  {
    // double size should be ok
    uint_t new_size = self->buffer->size + self->buffer->size;
    if (new_size < size) 
    {
      // more is always better
      new_size = size + self->buffer->size;
    }
    
    ecbuf_resize (self->buffer, new_size);  
    // set new position in possible new buffer
    self->pos = self->buffer->buffer + filled;      
  }
}

/*------------------------------------------------------------------------*/

void ecstream_appendd(EcStream self, const EcString source, uint_t size)
{
  _ecstream_check(self, size);
  
  // copy the source to the buffer
  memcpy(self->pos, source, size);
  
  self->pos = self->pos + size;
  // terminate
  *(self->pos) = 0;    
}

/*------------------------------------------------------------------------*/

void ecstream_append( EcStream self, const EcString source )
{
  // variables
  uint_t size = 0;

  // check the value
  if( ecstr_empty(source) )
  {
    return;  
  }
  // size of the source
  size = strlen(source);
  
  ecstream_appendd(self, source, size);
}

/*------------------------------------------------------------------------*/

void ecstream_appendc ( EcStream self, char c )
{
  _ecstream_check(self, 1);  
  // copy the source to the buffer
  *(self->pos) = c;
  (self->pos)++;
  // add termination at the end
  *(self->pos) = 0;   
}

/*------------------------------------------------------------------------*/

void ecstream_appendu( EcStream self, uint_t value )
{
  EcString h = ecstr_long(value);
  /* add */
  ecstream_append( self, h );
  
  ecstr_delete(&h);
}

//---------------------------------------------------------------------------------------

void ecstream_appendt (EcStream self, const time_t* value)
{
  char buffer [32];
#ifdef _WIN32
  _snprintf_s (buffer, 30, _TRUNCATE, "%lu", (unsigned long)*value);
#else
  snprintf(buffer, 30, "%lu", (unsigned long)*value);
#endif  
  ecstream_append (self, buffer);
}

/*------------------------------------------------------------------------*/

uint_t ecstream_size( EcStream self )
{
  return self->pos - self->buffer->buffer;
}

//---------------------------------------------------------------------------------------

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
  if (fh) {
    EcBuffer buffer = ecbuf_create (1024);

    uint_t res = ecfh_readBuffer(fh, buffer);
    while (res > 0)
    {
      ecdevstream_append (self, buffer->buffer, res);
      res = ecfh_readBuffer(fh, buffer);
    }
    
    ecfh_close(&fh);
    
    ecbuf_destroy (&buffer);    
  } else {
    ecdevstream_appends (self, "fle read error");
  }
}

//---------------------------------------------------------------------------------------

