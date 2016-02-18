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

#include "ecptrmap.h"

/*------------------------------------------------------------------------*/

struct EcPtrMap_s
{
  EcList list;
};

struct EcPtrMapDataNode
{
  void* key;
  
  void* data;
};


/*------------------------------------------------------------------------*/

EcPtrMap ecptrmap_create (EcAlloc alloc)
{
  EcPtrMap self = ECMM_NEW(struct EcPtrMap_s);
  
  self->list = eclist_create_ex (alloc);
  
  return self;
}

/*------------------------------------------------------------------------*/

void ecptrmap_destroy (EcAlloc alloc, EcPtrMap* pself)
{
  EcPtrMap self = *pself;
  
  eclist_free_ex (EC_ALLOC, &(self->list));
  
  ECMM_DEL(pself, struct EcPtrMap_s);
}

/*------------------------------------------------------------------------*/

void ecptrmap_clear(EcPtrMap self)
{
  //iterate through all list members
  EcListNode node;
  for(node = eclist_first(self->list); node != eclist_end(self->list); node = eclist_next(node))
  {
    struct EcPtrMapDataNode* mapnode = (struct EcPtrMapDataNode*)eclist_data(node);
    free(mapnode);
  }
  eclist_clear(self->list);    
}

/*------------------------------------------------------------------------*/

void ecptrmap_append(EcPtrMap self, void* key, void* data)
{
  struct EcPtrMapDataNode* mapnode = ENTC_NEW(struct EcPtrMapDataNode);
  mapnode->key = key;
  mapnode->data = data;
  
  eclist_append (EC_ALLOC, self->list, mapnode);  
}

/*------------------------------------------------------------------------*/

EcPtrMapNode ecptrmap_erase (EcPtrMap self, EcPtrMapNode node)
{
  struct EcPtrMapDataNode* mapnode = eclist_data(node);
  
  ENTC_DEL(&mapnode, struct EcPtrMapDataNode);
  
  return eclist_erase (EC_ALLOC, self->list, node);
}

/*------------------------------------------------------------------------*/

EcPtrMapNode ecptrmap_find(EcPtrMap self, void* key)
{
  //iterate through all list members
  EcListNode node;
  for(node = eclist_first(self->list); node != eclist_end(self->list); node = eclist_next(node))
  {
    struct EcPtrMapDataNode* mapnode = (struct EcPtrMapDataNode*)eclist_data(node);
    //compare the stored name with the name we got
    if(mapnode->key == key)
      return node;
  }
  return eclist_end(self->list);
}

/*------------------------------------------------------------------------*/

void* ecptrmap_finddata(EcPtrMap self, void* key)
{
  /* iterate through all list members */
  EcListNode node;
  for(node = eclist_first(self->list); node != eclist_end(self->list); node = eclist_next(node))
  {
    struct EcPtrMapDataNode* mapnode = (struct EcPtrMapDataNode*)eclist_data(node);
    /* compare the stored name with the name we got */
    if(mapnode->key == key)
      return mapnode->data;
  }
  return 0;    
}

/*------------------------------------------------------------------------*/

EcPtrMapNode ecptrmap_first(const EcPtrMap self)
{
  return eclist_first(self->list);  
}

/*------------------------------------------------------------------------*/

EcPtrMapNode ecptrmap_next(const EcPtrMapNode node)
{
  return eclist_next((EcListNode)node);  
}

/*------------------------------------------------------------------------*/

EcPtrMapNode ecptrmap_end(EcPtrMap self)
{
  return eclist_end(self->list);  
}

/*------------------------------------------------------------------------*/

void* ecptrmap_key(const EcPtrMapNode node)
{
  struct EcPtrMapDataNode* mapnode = eclist_data((EcListNode)node);
  return mapnode->key;  
}

/*------------------------------------------------------------------------*/

void* ecptrmap_data(const EcPtrMapNode node)
{
  struct EcPtrMapDataNode* mapnode = eclist_data((EcListNode)node);
  return mapnode->data;  
}

/*------------------------------------------------------------------------*/

void ecptrmap_reset(const EcPtrMapNode node, void* data)
{
  struct EcPtrMapDataNode* mapnode = eclist_data((EcListNode)node);
  mapnode->data = data;
}

/*------------------------------------------------------------------------*/

