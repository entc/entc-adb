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

#include "ecarray.h"
#include "ecvector.h"

#define ENTC_ARRAY_NODESIZE 10
#define ENTC_ARRAY_PARTSIZE 100

struct EcArray_s
{
  
  uint_t size;

  /* array of void* */
  void* *data;
  
};

/*------------------------------------------------------------------------*/

EcArray qcarray_new()
{
  EcArray self = ENTC_NEW(struct EcArray_s);
  
  self->size = 100;
  
  self->data = ENTC_MALLOC( sizeof(void*) * (self->size + 1) );
  
  memset(self->data, 0, (self->size + 1) * sizeof(void*) );
  
  return self;
}

/*------------------------------------------------------------------------*/

void qcarray_delete(EcArray* pself)
{
  EcArray self = *pself;
  
  ENTC_FREE( self->data );
  
  ENTC_DEL(pself, struct EcArray_s);
}

/*------------------------------------------------------------------------*/

void qcarray_clear(EcArray self)
{
  memset(self->data, 0, self->size);  
}

/*------------------------------------------------------------------------*/

void qcarray_resize(EcArray self, uint_t size)
{
  if( size != self->size )
  {
    free( self->data );
    
    self->size = size;
    
    self->data = malloc( sizeof(void*) * (self->size + 1) );
    
    memset(self->data, 0, (self->size + 1) * sizeof(void*) );
  }
}

/*------------------------------------------------------------------------*/

void qcarray_set(EcArray self, uint_t lindex, void* data)
{
  if( lindex < self->size )
  {
    (self->data)[lindex] = data;  
  }
}

/*------------------------------------------------------------------------*/

void* qcarray_get(EcArray self, uint_t lindex)
{
  if( lindex < self->size )
  {
    return (self->data)[lindex];
  }
  return 0;
}

/*------------------------------------------------------------------------*/
