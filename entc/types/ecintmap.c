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

#include "ecintmap.h"

/*------------------------------------------------------------------------*/

struct EcIntMap_s
{
  EcList list;
};

typedef struct
{
  uint_t key;
  
  void* data;
  
} EcIntMapDataNode;

/*------------------------------------------------------------------------*/

EcIntMap ecintmap_create (EcAlloc alloc)
{
  EcIntMap self = ECMM_NEW(struct EcIntMap_s);
  
  self->list = eclist_create_ex (alloc);
  
  return self;
}

/*------------------------------------------------------------------------*/

void ecintmap_destroy (EcAlloc alloc, EcIntMap* pself)
{
  EcIntMap self = *pself;
  
  ecintmap_clear(self);
  
  eclist_free_ex (EC_ALLOC, &(self->list));
  
  ECMM_DEL(pself, struct EcIntMap_s);
}

/*------------------------------------------------------------------------*/

void ecintmap_clear (EcIntMap self)
{
  /* iterate through all list members */
  EcListNode node;
  for(node = eclist_first(self->list); node != eclist_end(self->list); node = eclist_next(node))
  {
    EcIntMapDataNode* mapnode = eclist_data (node);
    
    ENTC_DEL (&mapnode, EcIntMapDataNode);
  }
  
  eclist_clear (self->list);    
}

/*------------------------------------------------------------------------*/

void ecintmap_append(EcIntMap self, uint_t key, void* data)
{
  EcIntMapDataNode* mapnode = ENTC_NEW (EcIntMapDataNode);
  mapnode->key = key;
  mapnode->data = data;
  
  eclist_append (EC_ALLOC, self->list, mapnode);  
}

/*------------------------------------------------------------------------*/

EcIntMapNode ecintmap_find (EcIntMap self, uint_t key)
{
  /* iterate through all list members */
  EcListNode node;
  for(node = eclist_first(self->list); node != eclist_end(self->list); node = eclist_next(node))
  {
    EcIntMapDataNode* mapnode = eclist_data(node);
    /* compare the stored name with the name we got */
    if(mapnode->key == key)
      return node;
  }
  return (EcIntMapNode)eclist_end(self->list);
}

/*------------------------------------------------------------------------*/

EcIntMapNode ecintmap_first(const EcIntMap self)
{
  return eclist_first(self->list);  
}

/*------------------------------------------------------------------------*/

EcIntMapNode ecintmap_next (const EcIntMapNode node)
{
  return eclist_next((EcListNode)node);  
}

/*------------------------------------------------------------------------*/

EcIntMapNode ecintmap_end(EcIntMap self)
{
  return eclist_end(self->list);  
}

/*------------------------------------------------------------------------*/

EcIntMapNode ecintmap_erase (EcIntMap self, EcIntMapNode node)
{
  EcIntMapDataNode* mapnode = eclist_data (node);
    
  ENTC_DEL (&mapnode, EcIntMapDataNode);
  
  return eclist_erase (EC_ALLOC, self->list, node);
}

/*------------------------------------------------------------------------*/

void* ecintmap_data(const EcIntMapNode node)
{
  EcIntMapDataNode* mapnode = eclist_data ((EcListNode)node);
  return mapnode->data;  
}

/*------------------------------------------------------------------------*/

void ecintmap_orderAll(EcIntMap self)
{
  /* iterate through all list members */
  EcListNode node01;
  EcListNode node02;

  for(node01 = eclist_first(self->list); node01 != eclist_end(self->list); node01 = eclist_next(node01))
  {
    EcIntMapDataNode* mapnode01 = eclist_data(node01);

    uint_t min_key = mapnode01->key;
    
    for(node02 = eclist_next(node01); node02 != eclist_end(self->list); node02 = eclist_next(node02))
    {
      EcIntMapDataNode* mapnode02 = eclist_data(node02);

      if( mapnode02->key < min_key )
      {
        /* switch both elements */
        void* data = mapnode02->data;
        mapnode02->data = mapnode01->data;
        mapnode01->data = data;
        
        min_key = mapnode02->key;
        mapnode02->key = mapnode01->key;
        mapnode01->key = min_key;
      }
    }
    
  }
}

/*------------------------------------------------------------------------*/
