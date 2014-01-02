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

#include "ectable.h"
#include <stdio.h>

/*------------------------------------------------------------------------*/

struct EcTable_s
{
  
  uint_t m_cols;
  
  uint_t m_rows;
  
  uint_t m_inds;
  
  uint_t m_fill;
  
  /* array of pointers */
  char** index;
  
};

typedef char* EcTableSegment; 

typedef char* EcTablePosition;

/*------------------------------------------------------------------------*/

EcTable ectable_new(uint_t cols, uint_t rows)
{
  EcTable self = ENTC_NEW(struct EcTable_s);
  
  self->m_cols = cols;
  
  if( rows > 0 )
  {
    self->m_rows = rows;
  }
  else
  {
    self->m_rows = 20;    
  }
  
  self->m_inds = 1;
  
  self->index = malloc(sizeof(char*) * self->m_inds);
  
  memset(self->index, 0, sizeof(char*) * self->m_inds);
  
  self->m_fill = 0;
  
  return self;
}

/*------------------------------------------------------------------------*/

void ectable_delete( EcTable* pself )
{
  EcTable self = *pself;
  
  ectable_clear( self );
  
  memset( self->index, 0, sizeof(char*) * self->m_inds );
  free( self->index );
  /* set zero */
  memset( self, 0, sizeof(struct EcTable_s) );
  
  ENTC_DEL(pself, struct EcTable_s);
}

/*------------------------------------------------------------------------*/

void ectable_clear( EcTable self )
{
  uint_t i;
  
  char** segptr = self->index;
  
  for(i = 0; i < self->m_inds; i++, segptr++ )
  {
    if( *segptr != 0 )
    {
      memset(*segptr, 0, self->m_cols * self->m_rows * sizeof(char*));
      free(*segptr);
      
    }
     
    *segptr = 0;
  }
}

/*------------------------------------------------------------------------*/

void ectable_resizeSegmentIndex( EcTable self, uint_t nsize )
{
  /* variables */
  uint_t nsize_bytes = nsize * sizeof(char*);
  uint_t osize_bytes = self->m_inds * sizeof(char*);

  self->index = realloc (self->index, nsize_bytes);

  memset(self->index + self->m_inds, 0x0, nsize_bytes - osize_bytes);

  self->m_inds = nsize;  
}

/*------------------------------------------------------------------------*/

EcTableSegment ectable_getSegment(EcTable self, uint_t segno)
{
  /* variables */
  EcTableSegment segment;  
  /* do we have a segment */
  if(segno > (self->m_inds - 1))
  {
    ectable_resizeSegmentIndex(self, segno + 1);
  }

  segment = self->index[segno];
  
  if(segment == 0)
  {
    uint_t size = self->m_cols * self->m_rows * sizeof(char*);
    
    segment = malloc( size );
    memset(segment, 0, size );
    
    self->index[segno] = segment;
  }
  
  return segment;
}

/*------------------------------------------------------------------------*/

EcTablePosition* ectable_getPosition(EcTable self, uint_t row, uint_t col)
{
  /* variables */
  uint_t segno;
  EcTableSegment segment;
  /* calculate the segment */
  segno = (row / self->m_rows);
  /* get the segment pointer */
  segment = ectable_getSegment(self, segno);
  /* get the position */
  return (EcTablePosition*)(segment + ((row - (segno * self->m_rows)) * self->m_cols + col) * sizeof(EcTablePosition));
}

/*------------------------------------------------------------------------*/

void ectable_set( EcTable self, uint_t row, uint_t col, void* value)
{
  EcTablePosition* position;
  
  if( col > self->m_cols )
  {
    return;  
  }

  position = ectable_getPosition(self, row, col); 
  
  *position = value;
  //printf("seg: %lu mrows: %lu row: %lu diff %lu pos : %lu\n", segment, self->m_rows, row, (row - segment * self->m_rows), pos);
  
  if (row > self->m_fill)
  {
    self->m_fill = row;
  }
}

/*------------------------------------------------------------------------*/

void* ectable_get( EcTable self, uint_t row, uint_t col )
{
  EcTablePosition* position;
  
  if( col > self->m_cols )
  {
    return 0;  
  }
  
  
  //printf("seg: %lu mrows: %lu row: %lu diff %lu pos : %lu\n", segment, self->m_rows, row, (row - segment * self->m_rows), pos);

  position = ectable_getPosition(self, row, col);    
  
  return *position;
}

/*------------------------------------------------------------------------*/

uint_t ectable_maxRows( EcTable self )
{
  return self->m_inds * self->m_rows;  
}

/*------------------------------------------------------------------------*/

EcTableNode ectable_first( EcTable self )
{
  return 0;
}

/*------------------------------------------------------------------------*/

EcTableNode ectable_end( EcTable self )
{
  if (self->m_fill == 0) 
  {
    return 0;
  }
  else
  {
    return self->m_fill + 1;
  }
}

/*------------------------------------------------------------------------*/

EcTableNode ectable_next( EcTable self, EcTableNode node )
{
  uint_t i;
  uint_t m = self->m_fill + 1; //ectable_maxRows( self );
  
  for(i = (node + 1); i < m; i++ )
  {
    uint_t segment = i / self->m_rows;
    
    char** segptr = &(self->index[segment]);
    
    if( *segptr != 0 )
    {
      return i;  
    }
  }
  
  return self->m_fill + 1;
}

/*------------------------------------------------------------------------*/

void* ectable_data( EcTable self, EcTableNode node, uint_t col )
{
  return ectable_get(self, node, col);
}

/*------------------------------------------------------------------------*/

uint_t ectable_getColumns( EcTable self )
{
  return self->m_cols;
}

/*------------------------------------------------------------------------*/

uint_t ectable_getRows( EcTable self )
{
  return self->m_fill;
}

/*------------------------------------------------------------------------*/
