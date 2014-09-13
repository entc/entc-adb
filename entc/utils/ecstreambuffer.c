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

#include "ecstreambuffer.h"

/*------------------------------------------------------------------------*/

struct EcStreamBuffer_s
{
  
  EcBuffer buffer;
  
  EcLogger logger;
  
  uint_t size;
  
  /* reference */
  EcSocket socket;
  
  unsigned char* pos;
  unsigned char* end;
  
};

/*------------------------------------------------------------------------*/

EcStreamBuffer ecstreambuffer_new (EcLogger logger, EcSocket socket)
{
  EcStreamBuffer self = ENTC_NEW(struct EcStreamBuffer_s);
  
  self->buffer = ecbuf_create (1024);
  self->logger = logger;
  self->socket = socket;
  self->pos = self->buffer->buffer;
  self->end = self->buffer->buffer;

  memset(self->buffer->buffer, 0x00, self->buffer->size );
  
  return self;
}

/*------------------------------------------------------------------------*/

void ecstreambuffer_delete (EcStreamBuffer* pself)
{
  EcStreamBuffer self = *pself;
  
  ecbuf_destroy (&(self->buffer));
  
  ENTC_DEL(pself, struct EcStreamBuffer_s);
}

/*------------------------------------------------------------------------*/

int ecstreambuffer_refill (EcStreamBuffer self, int* error)
{
  *error = ecsocket_readBunch (self->socket, self->buffer->buffer, self->buffer->size - 2);
  if( *error > 0 )
  {
    self->pos = self->buffer->buffer;
    self->end = self->pos + *error;
    /* create the cstring end */
    *(self->end) = 0;
    
    return TRUE;
  }
  else
  {
    self->pos = 0;
    self->end = 0;
    
    return FALSE;
  }
}

/*------------------------------------------------------------------------*/

int ecstreambuffer_next (EcStreamBuffer self, int* error)
{
  if( self->pos < self->end )
  {
    self->pos++;
  }
  else
  {
    /* refill the buffer */
    if( !ecstreambuffer_refill(self, error) )
    {
      return FALSE;  
    }
  }
  return TRUE;
}

/*------------------------------------------------------------------------*/

char ecstreambuffer_get (EcStreamBuffer self)
{
  return *(self->pos);  
}

/*------------------------------------------------------------------------*/

void* ecstreambuffer_buffer (EcStreamBuffer self)
{
  return self->pos;
}

/*------------------------------------------------------------------------*/

uint_t ecstreambuffer_filled (EcStreamBuffer self, int* error)
{
  /* variables */
  uint_t size = self->end - self->pos;
  /* check if it is zero */
  if( size == 0 )
  {
    ecstreambuffer_refill(self, error);
    /* calculate again */
    size = self->end - self->pos;
  }
  return size;
}

/*------------------------------------------------------------------------*/

uint_t ecstreambuffer_fill(EcStreamBuffer self, int* error)
{
  if( ecstreambuffer_refill(self, error) )
  {
    return self->end - self->pos;
  }
  return 0;
}

//---------------------------------------------------------------

int ecstreambuffer_readln_1break (EcStreamBuffer self, EcStream stream, int* error, char* b1, char* b2)
{  
  while (ecstreambuffer_next (self, error))
  {
    char c = *(self->pos);
    
    if (c == *b1)
    {
      return TRUE;
    }
    else if (c == '\r' || c == '\n')
    {
      // error, but we count it as line break
      return TRUE;
    }
    else
    {
      ecstream_appendc (stream, c);      
    }            
  }
  
  return FALSE;    
}

//---------------------------------------------------------------

int ecstreambuffer_readln_2breaks (EcStreamBuffer self, EcStream stream, int* error, char* b1, char* b2)
{
  int hasBreak1 = FALSE;
  
  while (ecstreambuffer_next (self, error))
  {
    char c = *(self->pos);
    
    if (hasBreak1)
    {
      if (c == *b2)
      {
        return TRUE;
      }
      else
      {        
        self->pos--;
        return TRUE;
      }
    }
    else
    {
      if (c == *b1)
      {
        hasBreak1 = TRUE;
      }
      else if (c == '\r' || c == '\n')
      {
        // error, but we count it as line break
        return TRUE;
      }
      else
      {
        ecstream_appendc (stream, c);      
      }                  
    }
  }
  
  return FALSE;  
}

//---------------------------------------------------------------

int ecstreambuffer_readln_getbreaks (EcStreamBuffer self, EcStream stream, int* error, char* b1, char* b2)
{
  int hasBreak1 = FALSE;
  
  while (ecstreambuffer_next (self, error))
  {
    char c = *(self->pos);
    
    if (hasBreak1)
    {
      if (c == *b1)
      {
        self->pos--;
        return TRUE;
      }
      else if (c == '\r' || c == '\n')
      {
        *b2 = c;
        return TRUE;
      }
      else
      {
        self->pos--;
        return TRUE;
      }
    }
    else
    {
      if (c == '\r' || c == '\n')
      {
        *b1 = c;
        hasBreak1 = TRUE;
        continue;
      }
      else
      {
        ecstream_appendc (stream, c);      
      }                  
    }
  }
  
  return FALSE;    
}

//---------------------------------------------------------------

int ecstreambuffer_readln (EcStreamBuffer self, EcStream stream, int* error, char* b1, char* b2)
{
  /* clean up the stream */
  ecstream_clear( stream );

  if (*b1 && *b2)
  {
    return ecstreambuffer_readln_2breaks (self, stream, error, b1, b2);
  }
  else if (*b1 && (*b2 == 0))
  {
    return ecstreambuffer_readln_1break (self, stream, error, b1, b2);
  }
  else
  {
    return ecstreambuffer_readln_getbreaks (self, stream, error, b1, b2);
  }
}

/*------------------------------------------------------------------------*/

void ecstreambuffer_read (EcStreamBuffer self, EcStream stream, int* error)
{
  /* clean up the stream */
  ecstream_clear( stream );
  
  while( ecstreambuffer_next (self, error) )
  {
    char c = ecstreambuffer_get(self);
    
    ecstream_appendc(stream, c);    
  }
}

/*------------------------------------------------------------------------*/

void* ecstreambuffer_getBunch (EcStreamBuffer self, ulong_t size, int* res)
{
  void* ret = NULL;

  if (ecstreambuffer_next (self, res))
  {
    uint_t filled = ecstreambuffer_filled (self, res);      
    if (filled > 0)
    {
      ulong_t diff = filled < size ? filled : size;
      
      ret = self->pos;
      
      self->pos += diff;
      
      *res = diff;
    }    
  }
  
  return ret;
}

//--------------------------------------------------------------------------
