#include "ecreadbuffer.h"

#include <string.h>

/*------------------------------------------------------------------------*/

struct EcReadBuffer_s
{
  EcFileHandle fhandle;
  EcBuffer buffer;
  
  unsigned char* max;
  unsigned char* pos;
  
  int close;

};

/*------------------------------------------------------------------------*/

EcReadBuffer ecreadbuffer_create (EcFileHandle fhandle, int lclose)
{
  EcReadBuffer self = ENTC_NEW(struct EcReadBuffer_s);
  
  self->fhandle = fhandle;
  self->buffer = ecbuf_create (1024);
  self->max = 0;
  self->pos = 0;
  self->close = lclose;
  
  return self;
}

/*------------------------------------------------------------------------*/

void ecreadbuffer_destroy (EcReadBuffer* pself)
{
  EcReadBuffer self = *pself;
  
  if( self->close )
  {
    ecfh_close( &(self->fhandle) );    
  }
  
  self->fhandle = 0;
  ecbuf_destroy (&(self->buffer));

  ENTC_DEL(pself, struct EcReadBuffer_s);
}

/*------------------------------------------------------------------------*/

int ecreadbuffer_getnext( EcReadBuffer self, char* character )
{
  if( self->fhandle )
  {
    /* check if the buffer is empty */
    if( self->max == self->pos )
    {
      uint_t res = ecfh_readBuffer(self->fhandle, self->buffer);
      /* no data */
      if( res == 0 )
      {
        return FALSE;
      }
      
      self->pos = self->buffer->buffer;
      self->max = self->buffer->buffer + res;
    }

    *character = *(self->pos);
    
    self->pos++;

    return TRUE;
  }
  return FALSE;  
}

/*------------------------------------------------------------------------*/

uint_t ecreadbuffer_get( EcReadBuffer self, uint_t size )
{
  if( self->fhandle )
  {
    uint_t diff = self->max - self->pos;
    
    while (diff < size )
    {
	  uint_t res;
	  EcBuffer_s diffbuffer;

	  if (self->buffer->buffer != self->pos)
	  {
        // move the rest of the buffer to the start of the buffer
        memmove ( self->buffer->buffer, self->pos, diff );

        self->pos = self->buffer->buffer;      
	  }
    
      // read from file
	  diffbuffer.buffer = self->buffer->buffer + diff;
	  diffbuffer.size = self->buffer->size - diff;

      res = ecfh_readBuffer(self->fhandle, &diffbuffer);
      if (res < 1)
	  {
		self->max = diffbuffer.buffer;
		break;
	  }

	  self->max = diffbuffer.buffer + res;
      
      diff = diff + res;
    }
    
    return diff;
  }
  
  return 0;
}

/*------------------------------------------------------------------------*/

const EcString ecreadbuffer_buffer( EcReadBuffer self )
{
  return (EcString)self->pos;  
}

/*------------------------------------------------------------------------*/
