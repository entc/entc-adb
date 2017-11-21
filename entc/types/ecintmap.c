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

//----------------------------------------------------------------------------------------

static int __STDCALL ecintmap_onDestroy (void* ptr)
{
  EcIntMapDataNode* mapnode = ptr;
  
  ENTC_DEL (&mapnode, EcIntMapDataNode);
  
  return 0;
}

/*------------------------------------------------------------------------*/

EcIntMap ecintmap_create (EcAlloc alloc)
{
  EcIntMap self = ECMM_NEW(struct EcIntMap_s);
  
  self->list = eclist_create (ecintmap_onDestroy);
  
  return self;
}

/*------------------------------------------------------------------------*/

void ecintmap_destroy (EcAlloc alloc, EcIntMap* pself)
{
  EcIntMap self = *pself;
  
  ecintmap_clear(self);
  
  eclist_destroy (&(self->list));
  
  ECMM_DEL(pself, struct EcIntMap_s);
}

/*------------------------------------------------------------------------*/

void ecintmap_clear (EcIntMap self)
{
  eclist_clear (self->list);
}

/*------------------------------------------------------------------------*/

void ecintmap_append(EcIntMap self, uint_t key, void* data)
{
  EcIntMapDataNode* mapnode = ENTC_NEW (EcIntMapDataNode);
  mapnode->key = key;
  mapnode->data = data;
  
  eclist_push_back (self->list, mapnode);
}

/*------------------------------------------------------------------------*/

EcIntMapNode ecintmap_find (EcIntMap self, uint_t key)
{
  EcListCursor cursor;
  
  eclist_cursor_init (self->list, &cursor, LIST_DIR_NEXT);
  
  while (eclist_cursor_next (&cursor))
  {
    EcIntMapDataNode* mapnode = eclist_data (cursor.node);

    if (mapnode->key == key)
    {
      return cursor.node;
    }
  }
  
  return NULL;
}

/*------------------------------------------------------------------------*/

EcIntMapNode ecintmap_first (const EcIntMap self)
{
  EcListCursor cursor;
  
  eclist_cursor_init (self->list, &cursor, LIST_DIR_NEXT);
  
  if (eclist_cursor_next (&cursor))
  {
    return cursor.node;
  }
  else
  {
    return NULL;
  }
}

/*------------------------------------------------------------------------*/

EcIntMapNode ecintmap_next (const EcIntMapNode node)
{
  return eclist_next (node);
}

/*------------------------------------------------------------------------*/

EcIntMapNode ecintmap_end (EcIntMap self)
{
  return NULL;
}

/*------------------------------------------------------------------------*/

uint_t ecintmap_key (const EcIntMapNode node)
{
  EcIntMapDataNode* mapnode = eclist_data (node);

  return mapnode->key;
}

/*------------------------------------------------------------------------*/

EcIntMapNode ecintmap_erase (EcIntMap self, EcIntMapNode node)
{
  EcListCursor cursor;
  
  cursor.node = node;
  
  eclist_cursor_erase (self->list, &cursor);
  
  return cursor.node;
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
  EcListCursor cursor01;
  
  eclist_cursor_init (self->list, &cursor01, LIST_DIR_NEXT);
  
  while (eclist_cursor_next (&cursor01))
  {
    EcIntMapDataNode* mapnode01 = eclist_data(cursor01.node);

    uint_t min_key = mapnode01->key;
    
    EcListCursor cursor02;

    cursor02.node = cursor01.node;
    cursor02.position = cursor01.position;
    cursor02.direction = cursor01.direction;
    
    while (eclist_cursor_next (&cursor02))
    {
      EcIntMapDataNode* mapnode02 = eclist_data(cursor02.node);

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
