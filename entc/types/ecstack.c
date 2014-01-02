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

#include "ecstack.h"

/*------------------------------------------------------------------------*/

struct EcStackNode
{
  
  void* data;
  
  struct EcStackNode* pref;
  
};

struct EcStack_s
{
  
  struct EcStackNode* top;
  
};

/*------------------------------------------------------------------------*/

EcStack ecstack_new()
{
  EcStack self = ENTC_NEW(struct EcStack_s);
  
  self->top = 0;
  
  return self;
}

/*------------------------------------------------------------------------*/

void ecstack_delete(EcStack* pself)
{
  EcStack self = *pself;
  
  ecstack_clear(self);
  
  ENTC_DEL(pself, struct EcStack_s);
}

/*------------------------------------------------------------------------*/

void ecstack_clear(EcStack self)
{
  while( ecstack_pop(self) == TRUE );
}

/*------------------------------------------------------------------------*/

void ecstack_push(EcStack self, void* data)
{
  struct EcStackNode* node = ENTC_NEW(struct EcStackNode);
  
  node->data = data;
  node->pref = self->top;
  
  self->top = node;
}

/*------------------------------------------------------------------------*/

int ecstack_pop(EcStack self)
{
  struct EcStackNode* node = self->top;
  
  if( node )
  {
    self->top = node->pref;
    
    ENTC_DEL(&node, struct EcStackNode);
    
    return TRUE;
  }
  return FALSE;
}

/*------------------------------------------------------------------------*/

void* ecstack_top(EcStack self)
{
  struct EcStackNode* node = self->top;
  
  if( node )
  {
    return node->data;
  }
  return 0;
}

/*------------------------------------------------------------------------*/
