/*
 * Copyright (c) 2010-2016 "Alexander Kalkhof" [email:entc@kalkhof.org]
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

#include "ecmap.h"

// TODO: this object must be review in respect of using a real hashtable

//----------------------------------------------------------------------------------------

struct EcMap_s
{
  EcList list;
};

struct EcMapDataNode
{
  EcString key;
  
  void* data;
};

//----------------------------------------------------------------------------------------

static int __STDCALL ecmap_onDestroy (void* ptr)
{
  struct EcMapDataNode* mapnode = ptr;
  
  ecstr_delete(&(mapnode->key));
  
  ENTC_DEL(&mapnode, struct EcMapDataNode);
  
  return 0;
}

//----------------------------------------------------------------------------------------

EcMap ecmap_create (EcAlloc alloc)
{
  EcMap self = ECMM_NEW(struct EcMap_s);

  self->list = eclist_create (ecmap_onDestroy);

  return self;
}

//----------------------------------------------------------------------------------------

void ecmap_clear (EcAlloc alloc, EcMap self)
{
  eclist_clear (self->list);
}

//----------------------------------------------------------------------------------------

void ecmap_destroy (EcAlloc alloc, EcMap* pself)
{
  EcMap self = *pself;
  
  // clear all elements
  ecmap_clear (alloc, self);

  // delete the list
  eclist_destroy (&(self->list));
  
  ECMM_DEL(pself, struct EcMap_s);
}

//----------------------------------------------------------------------------------------

EcMapNode ecmap_append (EcMap self, const EcString key, void* data)
{
  struct EcMapDataNode* mapnode = ENTC_NEW(struct EcMapDataNode);
  mapnode->key = ecstr_copy(key);
  mapnode->data = data;
  
  return eclist_push_back (self->list, mapnode);
}

//----------------------------------------------------------------------------------------

EcMapNode ecmap_erase (EcMap self, EcMapNode node)
{
  EcListCursor cursor;

  cursor.node = node;
  
  eclist_erase (self->list, &cursor);
  
  return cursor.node;
}

//----------------------------------------------------------------------------------------

EcMapNode ecmap_find (EcMap self, const EcString key)
{
  EcListCursor cursor;
  
  eclist_cursor_init (self->list, &cursor, LIST_DIR_NEXT);
  
  while (eclist_cursor_next (&cursor))
  {
    struct EcMapDataNode* mapnode = eclist_data(cursor.node);
    
    //compare the stored name with the name we got
    if(ecstr_equal(mapnode->key, key))
    {
      return cursor.node;
    }
  }
  
  return NULL;
}

//----------------------------------------------------------------------------------------

EcMapNode ecmap_first (const EcMap self)
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

//----------------------------------------------------------------------------------------

EcMapNode ecmap_end (const EcMap self)
{
  return NULL;
}

//----------------------------------------------------------------------------------------

EcMapNode ecmap_next (const EcMapNode node)
{
  return eclist_next (node);
}

//----------------------------------------------------------------------------------------

EcString ecmap_key (const EcMapNode node)
{
  struct EcMapDataNode* mapnode = eclist_data((EcListNode)node);
  return mapnode->key;  
}

//----------------------------------------------------------------------------------------

void* ecmap_data (const EcMapNode node)
{
  struct EcMapDataNode* mapnode = eclist_data((EcListNode)node);
  return mapnode->data;
}

//----------------------------------------------------------------------------------------

uint_t ecmap_size (const EcMap self)
{
  return eclist_size(self->list);
}

//----------------------------------------------------------------------------------------

void ecmap_cursor (EcMap self, EcMapCursor* cursor)
{
  cursor->map = self;
  cursor->node = ecmap_first (self);
}

//----------------------------------------------------------------------------------------

int ecmap_cnext (EcMapCursor* cursor)
{
  cursor->node = ecmap_next(cursor->node);
  return cursor->node != ecmap_end (cursor->map);
}

//----------------------------------------------------------------------------------------
