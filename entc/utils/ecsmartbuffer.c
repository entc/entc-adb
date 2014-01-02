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

#include "ecsmartbuffer.h"
#include "../utils/ecsecfile.h"

struct EcSmartbuffer_s
{  
  EcFileHandle fhandle;
  
  EcBuffer buffer;
  
  unsigned char* bend;
  
  unsigned char* bpos;
  
  uint_t bmax;
  
  
  EcString ckey;
  
  uint_t csize;
  
  int compare;
  
  uint_t cpos;
  
};

/*------------------------------------------------------------------------*/

EcSmartbuffer ecsmartbuffer_new(uint_t max)
{
  EcSmartbuffer self = ENTC_NEW( struct EcSmartbuffer_s );
  
  self->fhandle = 0;
  self->buffer = ecstr_buffer(max);
  self->bend = self->buffer->buffer + max;
  self->bpos = self->buffer->buffer;
  self->bmax = max;
  self->ckey = 0;
  self->csize = 0;
  self->compare = FALSE;
  self->cpos = 0;  
  
  return self;
}

/*------------------------------------------------------------------------*/

void ecsmartbuffer_delete(EcSmartbuffer self)
{
  /* ecsmartbuffer_flush(this, 0); */
  
  if(self->ckey) free(self->ckey);

  if(self->fhandle)
  {
    ecfh_close( &(self->fhandle) );
	self->fhandle = 0;
  }

  ecstr_release(&(self->buffer));
  
  free(self);
}

/*------------------------------------------------------------------------*/

void ecsmartbuffer_setFile(EcSmartbuffer self, const EcString filename, const EcString confdir, EcLogger logger)
{
  if(self->fhandle)
  {
    ecfh_close( &(self->fhandle) );
  }
  
  if(filename)
  {
    struct EcSecFopen secopen;
    if( ecsec_fopen(&secopen, filename, O_WRONLY | O_CREAT | O_TRUNC, logger, confdir) )
    {
      self->fhandle = secopen.fhandle;
      
      ecstr_delete( &(secopen.filename) );
    }
  }
}

/*------------------------------------------------------------------------*/

void ecsmartbuffer_append(EcSmartbuffer self, const EcString start, uint_t length, uint_t keep)
{
	//Variable definition
	uint_t diff01 = length;
	uint_t offset = 0;

	if(length == 0) return;

	while((self->bpos + diff01) > self->bend)
	{
		//get the difference between the end and the current pos
		uint_t diff02 = self->bend - self->bpos;   

		memcpy( self->bpos, start + offset, diff02 * sizeof(char) );

		self->bpos = self->bpos + diff02;
		*(self->bpos) = 0;

		offset = offset + diff02;

		ecsmartbuffer_flush(self, keep);

		diff01 = diff01 - diff02;
	}

	memcpy( self->bpos, start + offset, diff01 * sizeof(char) );

	self->bpos = self->bpos + diff01;
	*(self->bpos) = 0;
}

/*------------------------------------------------------------------------*/

void ecsmartbuffer_flush(EcSmartbuffer self, uint_t keep)
{
  if(keep == 0)
  {
    if( self->fhandle )
    {
      //dump it all into the file
	  ecfh_writeBuffer(self->fhandle, self->buffer, self->bpos - self->buffer->buffer);
    }
    self->bpos = self->buffer->buffer;
    *(self->bpos) = 0;
  }
  else
  //flush only except the last keep characters
  {
    EcString h = (char*)malloc( sizeof(char) * (keep + 1));
    self->bpos = self->bpos - keep;
    
    memcpy(h, self->bpos, keep * sizeof(char) );
    
    *(self->bpos) = 0;
    //do the flush stuff
    if( self->fhandle )
    {
      //dump it all into the file
      ecfh_writeBuffer(self->fhandle, self->buffer, self->bpos - self->buffer->buffer);
    }
    
    memcpy(self->buffer->buffer, h, keep * sizeof(char) );
    
    self->bpos = self->buffer->buffer + keep;
    *(self->bpos) = 0;
    
    free(h);
  }
}

/*------------------------------------------------------------------------*/

char* ecsmartbuffer_data(EcSmartbuffer self, uint_t n)
{
  if(n)
  {
    return (char*)self->bpos - n;
  }
  return (char*)self->buffer->buffer;
}

/*------------------------------------------------------------------------*/

EcString ecsmartbuffer_copy(EcSmartbuffer self)
{
  return ecstr_copy(ecstr_get(self->buffer));
}

/*------------------------------------------------------------------------*/

void ecsmartbuffer_reduce(EcSmartbuffer self, uint_t n)
{
  uint_t diff = self->bpos - self->buffer->buffer;
  if(n <= diff)
  {
    self->bpos = self->bpos - n;
    *(self->bpos) = 0;
  }
///  else
///    QLOG_ERROR(_logger, "[SBUF]", "try to reduce more (" << n << ") then available (" << diff << ")" );
}

/*------------------------------------------------------------------------*/

int ecsmartbuffer_isEmpty(EcSmartbuffer self)
{
  return self->bpos == self->buffer->buffer;
}

/*------------------------------------------------------------------------*/

int ecsmartbuffer_isEqual(EcSmartbuffer self, const EcString to)
{
  return ecstr_equal(to, (char*)self->buffer->buffer);
}

/*------------------------------------------------------------------------*/

void ecsmartbuffer_setCompareKey(EcSmartbuffer self, const EcString key)
{
  if(self->ckey)
  {
    free(self->ckey);
    self->ckey = 0;
    self->csize = 0;
  }
  
  ecsmartbuffer_flush(self, 0);
  self->cpos = 0;
  
  if(key)
  {
    self->ckey = ecstr_copy (key);
    self->csize = ecstr_len (key);
  }
  
  self->compare = TRUE;
}

/*------------------------------------------------------------------------*/

uint_t ecsmartbuffer_getCompareStatus(EcSmartbuffer self)
{
  uint_t left = self->csize - self->cpos;
  
  unsigned char* pos = self->buffer->buffer + self->cpos;
  
  while(*pos && left)
  {
    if(*pos != self->ckey[self->cpos])
    {
      self->compare = FALSE;
      return 0;
    }
    pos++;
    self->cpos++;
    left--;
  }
  return left;
}

/*------------------------------------------------------------------------*/

int ecsmartbuffer_getCompareEqual(EcSmartbuffer self)
{
  return self->compare;
}

/*------------------------------------------------------------------------*/
