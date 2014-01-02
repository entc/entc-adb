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

struct EcIntMapDataNode
{
  uint_t key;
  
  void* data;
};

/*------------------------------------------------------------------------*/

EcIntMap ecintmap_new()
{
  EcIntMap self = ENTC_NEW(struct EcIntMap_s);
  
  self->list = eclist_new();
  
  return self;
}

/*------------------------------------------------------------------------*/

void ecintmap_delete(EcIntMap* pself)
{
  EcIntMap self = *pself;
  
  ecintmap_clear(self);
  
  eclist_delete( &(self->list) );
  
  ENTC_DEL(pself, struct EcIntMap_s);
}

/*------------------------------------------------------------------------*/

void ecintmap_clear(EcIntMap self)
{
  /* iterate through all list members */
  EcListNode node;
  for(node = eclist_first(self->list); node != eclist_end(self->list); node = eclist_next(node))
  {
    struct EcIntMapDataNode* mapnode = eclist_data(node);
    free(mapnode);
  }
  eclist_clear(self->list);    
}

/*------------------------------------------------------------------------*/

void ecintmap_append(EcIntMap self, uint_t key, void* data)
{
  struct EcIntMapDataNode* mapnode = ENTC_NEW(struct EcIntMapDataNode);
  mapnode->key = key;
  mapnode->data = data;
  
  eclist_append(self->list, mapnode);  
}

/*------------------------------------------------------------------------*/

EcIntMapNode ecintmap_find(EcIntMap self, uint_t key)
{
  /* iterate through all list members */
  EcListNode node;
  for(node = eclist_first(self->list); node != eclist_end(self->list); node = eclist_next(node))
  {
    struct EcIntMapDataNode* mapnode = eclist_data(node);
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

EcIntMapNode
ecintmap_next(const EcIntMapNode node)
{
  return eclist_next((EcListNode)node);  
}

/*------------------------------------------------------------------------*/

EcIntMapNode ecintmap_end(EcIntMap self)
{
  return eclist_end(self->list);  
}

/*------------------------------------------------------------------------*/

void* ecintmap_data(const EcIntMapNode node)
{
  struct EcIntMapDataNode* mapnode = eclist_data((EcListNode)node);
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
    struct EcIntMapDataNode* mapnode01 = eclist_data(node01);

    uint_t min_key = mapnode01->key;
    
    for(node02 = eclist_next(node01); node02 != eclist_end(self->list); node02 = eclist_next(node02))
    {
      struct EcIntMapDataNode* mapnode02 = eclist_data(node02);

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
