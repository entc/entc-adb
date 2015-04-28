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

#include "eclist.h"
#include <stdlib.h>

struct EcListNode_s
{
  void* data;
  EcListNode next;
  EcListNode forw;
};

struct EcList_s
{
  //next == begin
  //forw == end
  EcListNode node;  
};

/*------------------------------------------------------------------------*/

EcList eclist_new()
{
  EcList self;
  
  self = ENTC_NEW( struct EcList_s );

  self->node = ENTC_NEW( struct EcListNode_s );
  /* do the loop */
  self->node->next = self->node;
  self->node->forw = self->node;
  
  return self;
}

/*------------------------------------------------------------------------*/

void eclist_delete(EcList* pself)
{
  EcList self = *pself;
  
  eclist_clear(self);
  
  ENTC_DEL(&(self->node), struct EcListNode_s);
  
  ENTC_DEL(pself, struct EcList_s);
}

/*------------------------------------------------------------------------*/

EcListNode eclist_insert(EcListNode node, void* data)
{
  EcListNode noden = ENTC_NEW(struct EcListNode_s);
  EcListNode n01;
  /* set the data */
  noden->data = data;

  n01 = node->next;
  /* set forward */
  noden->forw = n01->forw;
  n01->forw = noden;
  /* set next */
  node->next = noden;
  noden->next = n01;
  
  return noden;
}

/*------------------------------------------------------------------------*/

EcListNode eclist_append(EcList self, void* data)
{
  return eclist_insert(self->node->forw, data);
}

/*------------------------------------------------------------------------*/

void eclist_set(EcListNode node, void* data)
{
  node->data = data;
}

/*------------------------------------------------------------------------*/

EcListNode eclist_erase(EcListNode node)
{
  EcListNode nextn = node->next;
  EcListNode forwn = node->forw;
  
  forwn->next = nextn;
  nextn->forw = forwn;  
  
  free(node);
  
  return forwn;
}

/*------------------------------------------------------------------------*/

EcListNode eclist_first(const EcList self)
{
  return self->node->next;
}

/*------------------------------------------------------------------------*/

EcListNode eclist_last(const EcList self)
{
  return self->node->forw;
}

/*------------------------------------------------------------------------*/

EcListNode eclist_end(const EcList self)
{
  return self->node;
}

/*------------------------------------------------------------------------*/

EcListNode eclist_next(const EcListNode node)
{
  return node->next;
}

/*------------------------------------------------------------------------*/

EcListNode eclist_back(const EcListNode node)
{
  return node->forw;
}

/*------------------------------------------------------------------------*/

void* eclist_data(const EcListNode node)
{
  return node->data;
}

/*------------------------------------------------------------------------*/

void eclist_clear(EcList self)
{
  EcListNode node = self->node->next;
  
  while(node != self->node)
  {
    EcListNode node_del = node;
    
    node = node->next;
    
    free(node_del);
    
    node_del = 0;
  }
  
  self->node->next = self->node;
  self->node->forw = self->node;
}

/*------------------------------------------------------------------------*/

uint_t eclist_size(const EcList self)
{
  uint_t ret = 0;
  
  EcListNode node = self->node->next;
  
  while(node != self->node)
  {
    ret++;
    
    node = node->next;
  }
  
  return ret;
}

/*------------------------------------------------------------------------*/

void eclist_remove(EcList self, void* data)
{
  EcListNode node = self->node->next;
  
  while(node != self->node)
  {
    if(node->data == data)
    {
      eclist_erase(node);
      return;
    }
    node = node->next;
  }
}

/*------------------------------------------------------------------------*/

void eclist_cursor (EcList self, EcListCursor* cursor)
{
  cursor->list = self;
  cursor->node = self->node->next;
  cursor->value = NULL;
}

/*------------------------------------------------------------------------*/

int eclist_cnext (EcListCursor* cursor)
{
  cursor->node = cursor->node->next;
  cursor->value = cursor->node->data;
  return cursor->node != cursor->list->node;
}

/*------------------------------------------------------------------------*/


