/*
 * Copyright (c) 2010-2015 "Alexander Kalkhof" [email:entc@kalkhof.org]
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

#include "eccursor.h"

#include "ectable.h"
#include "utils/eclogger.h"

struct EcCursor_s
{
  
  int pos;
  
  int max;
  
  void* ptr;
  
  eccursor_fill_fct fill;
  
  eccursor_destroy_fct dest;
  
  EcTable data;
  
};

//----------------------------------------------------------------------------------------

EcCursor eccursor_create (void)
{
  EcCursor self = ENTC_NEW (struct EcCursor_s);
  
  self->pos = 0;
  self->max = 0;
    
  self->ptr = NULL;
  self->fill = NULL;
  self->dest = NULL;
  
  return self;
}

//----------------------------------------------------------------------------------------

void eccursor_destroy (EcCursor* pself)
{
  EcCursor self = *pself;
  
  if (isAssigned (self->dest))
  {
    self->dest (self->ptr, self->data);
  }
  
  ectable_delete (&(self->data));
  
  ENTC_DEL (pself, struct EcCursor_s);
}

//----------------------------------------------------------------------------------------

int eccursor_fill (EcCursor self)
{
  if (isNotAssigned (self->fill))
  {
    return FALSE;
  }
  
  self->max = self->fill (self->ptr, &(self->data));
  
  eclogger_fmt (LL_TRACE, "ENTC", "cursor fill", "cursor refilled with max %i", self->max);
  
  if (self->max > 0)
  {
    self->pos = 0;    
    return TRUE;
  }
  else
  {
    return FALSE;
  }
}

//----------------------------------------------------------------------------------------

int eccursor_next (EcCursor self)
{
  self->pos++;

  if (self->pos < self->max)
  {
    return TRUE;
  }
  else
  {
    // need to fill again
    return eccursor_fill (self);
  }  
}

//----------------------------------------------------------------------------------------

void* eccursor_get (EcCursor self, int column)
{
  return ectable_get (self->data, self->pos + 1, column);
}

//----------------------------------------------------------------------------------------

int eccursor_cols (EcCursor self)
{
  return ectable_getColumns (self->data);
}

//----------------------------------------------------------------------------------------

void* eccursor_header (EcCursor self, int column)
{
  // header is always the first row
  return ectable_get (self->data, 0, column);  
}

//----------------------------------------------------------------------------------------

void eccursor_callbacks (EcCursor self, void* ptr, eccursor_fill_fct fillfct, eccursor_destroy_fct destfct)
{
  self->ptr = ptr;
  self->fill = fillfct;
  self->dest = destfct;
}

//----------------------------------------------------------------------------------------

