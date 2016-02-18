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

#include "ecvector.h"

/*------------------------------------------------------------------------*/

struct EcVector_s
{
  EcList list;
  
  uint_t indexcounter;
  
};

struct EcVectorDataNode
{
  uint_t index;
  
  void* data;
};

/*------------------------------------------------------------------------*/

EcVector ecvector_create (EcAlloc alloc)
{
  EcVector self = ENTC_NEW(struct EcVector_s);
  
  self->list = eclist_create_ex (alloc);
  
  self->indexcounter = 0;
  
  return self;  
}

/*------------------------------------------------------------------------*/

void ecvector_destroy (EcAlloc alloc, EcVector* pself)
{
  EcVector self = *pself;
  
  ecvector_clear( self );
  
  eclist_free_ex (EC_ALLOC, &(self->list));

  ENTC_DEL(pself, struct EcVector_s);
}

/*------------------------------------------------------------------------*/

EcVectorNode ecvector_append(EcVector self, void* data)
{
  struct EcVectorDataNode* vecnode = ENTC_NEW(struct EcVectorDataNode);
  
  self->indexcounter++;
  
  vecnode->data = data;
  vecnode->index = self->indexcounter;
  
  return (EcVectorNode)eclist_append_ex (EC_ALLOC, self->list, vecnode);
}

/*------------------------------------------------------------------------*/

uint_t ecvector_add(EcVector self, void* data)
{
  struct EcVectorDataNode* vecnode = ENTC_NEW(struct EcVectorDataNode);
  
  self->indexcounter++;
  
  vecnode->data = data;
  vecnode->index = self->indexcounter;
  
  eclist_append_ex (EC_ALLOC, self->list, vecnode);
  
  return self->indexcounter;
}

/*------------------------------------------------------------------------*/

EcVectorNode ecvector_find(EcVector self, uint_t lindex)
{
  /* TODO: Implement a real index here */
  EcListNode node;
  for(node = eclist_first(self->list); node != eclist_end(self->list); node = eclist_next(node))
  {
    struct EcVectorDataNode* vecnode = (struct EcVectorDataNode*)eclist_data(node);
    if(vecnode->index == lindex)
      return node;
  }
  return 0;  
}

/*------------------------------------------------------------------------*/

void* ecvector_get(EcVector self, uint_t lindex)
{
  /* TODO: Implement a real index here */
  EcListNode node;
  for(node = eclist_first(self->list); node != eclist_end(self->list); node = eclist_next(node))
  {
    struct EcVectorDataNode* vecnode = (struct EcVectorDataNode*)eclist_data(node);
    if(vecnode->index == lindex)
      return vecnode->data;
  }
  return 0;  
}

/*------------------------------------------------------------------------*/

EcVectorNode ecvector_at(const EcVector self, uint_t lindex)
{
  EcListNode node;
  for(node = eclist_first(self->list); node != eclist_end(self->list); node = eclist_next(node))
  {
    struct EcVectorDataNode* vecnode = eclist_data(node);
        
    if(vecnode->index == lindex)
    {
      return node;
    }
  }
  
  return eclist_end(self->list);
}

/*------------------------------------------------------------------------*/

EcVectorNode ecvector_erase (EcVector self, EcVectorNode node)
{
  struct EcVectorDataNode* vecnode = (struct EcVectorDataNode*)eclist_data(node);

  ENTC_DEL(&vecnode, struct EcVectorDataNode);
  
  return eclist_erase (EC_ALLOC, self->list, node);  
}

/*------------------------------------------------------------------------*/

EcVectorNode ecvector_first(const EcVector self)
{
  return eclist_first(self->list);  
}

/*------------------------------------------------------------------------*/

EcVectorNode ecvector_last(const EcVector self)
{
  return eclist_last(self->list);
}

/*------------------------------------------------------------------------*/

EcVectorNode ecvector_end(const EcVector self)
{
  return eclist_end(self->list);
}

/*------------------------------------------------------------------------*/

EcVectorNode ecvector_next(const EcVectorNode node)
{
  return eclist_next((EcListNode)node);
}

/*------------------------------------------------------------------------*/

EcVectorNode ecvector_back(const EcVectorNode node)
{
  return eclist_back((EcListNode)node);  
}

/*------------------------------------------------------------------------*/

void* ecvector_data(const EcVectorNode node)
{
  struct EcVectorDataNode* vecnode = (struct EcVectorDataNode*)eclist_data((EcListNode)node);
  
  return vecnode->data;
}

/*------------------------------------------------------------------------*/

uint_t ecvector_index(const EcVectorNode node)
{
  struct EcVectorDataNode* vecnode = (struct EcVectorDataNode*)eclist_data((EcListNode)node);

  return vecnode->index;
}

/*------------------------------------------------------------------------*/

void ecvector_clear(EcVector self)
{
  /* iterate through all list members */
  EcListNode node;
  for(node = eclist_first(self->list); node != eclist_end(self->list); node = eclist_next(node))
  {
    struct EcVectorDataNode* vecnode = (struct EcVectorDataNode*)eclist_data(node);
    
    free(vecnode);
  }
  eclist_clear(self->list);
  
  self->indexcounter = 0;
}

/*------------------------------------------------------------------------*/

uint_t ecvector_size(EcVector self)
{
  EcListNode node;

  uint_t size = 0;
  
  for(node = eclist_first(self->list); node != eclist_end(self->list); node = eclist_next(node))
  {
    size++;
  }
  
  return size;
}

/*------------------------------------------------------------------------*/
