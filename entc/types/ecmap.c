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

#include <string.h>

#include "tools/eclog.h"

// TODO: this object must be review in respect of using a real hashtable

//----------------------------------------------------------------------------------------

struct EcMapNode_s
{

  EcMap map;

  void* key;

  void* val;

};

//----------------------------------------------------------------------------------------

struct EcMap_s
{
  EcList list;

  fct_ecmap_cmp onCompare;

  fct_ecmap_destroy onDestroy;
};

//----------------------------------------------------------------------------------------

void* ecmap_node_value (EcMapNode node)
{
  return node->val;
}

//----------------------------------------------------------------------------------------

void* ecmap_node_key (EcMapNode node)
{
  return node->key;
}

//----------------------------------------------------------------------------------------

void* ecmap_node_extract (EcMapNode node)
{
  void* val = node->val;

  // transfer ownership
  node->val = NULL;

  return val;
}

//----------------------------------------------------------------------------------------

void ecmap_node_destroy (EcMapNode* pnode)
{
  ENTC_DEL(pnode, struct EcMapNode_s);
}

//----------------------------------------------------------------------------------------

static int __STDCALL ecmap_onDestroy (void* ptr)
{
  EcMapNode node = ptr;

  ecmap_destroy_node (node->map, &node);

  return 0;
}

//-----------------------------------------------------------------------------

static int __STDCALL ecmap_node_cmp (const void* a, const void* b)
{
  return strcmp(a, b);
}

//----------------------------------------------------------------------------------------

EcMap ecmap_create (fct_ecmap_cmp onCompare, fct_ecmap_destroy onDestroy)
{
  EcMap self = ENTC_NEW(struct EcMap_s);

  self->list = eclist_create (ecmap_onDestroy);

  self->onCompare = onCompare ? onCompare : ecmap_node_cmp;
  self->onDestroy = onDestroy;

  return self;
}

//----------------------------------------------------------------------------------------

void ecmap_clear (EcMap self)
{
  eclist_clear (self->list);
}

//----------------------------------------------------------------------------------------

void ecmap_destroy (EcMap* pself)
{
  EcMap self = *pself;

  // clear all elements
  ecmap_clear (self);

  // delete the list
  eclist_destroy (&(self->list));

  ENTC_DEL(pself, struct EcMap_s);
}

//----------------------------------------------------------------------------------------

void ecmap_setcmp (EcMap self, fct_ecmap_cmp onCompare)
{
  self->onCompare = onCompare ? onCompare : ecmap_node_cmp;
}

//----------------------------------------------------------------------------------------

EcMapNode ecmap_insert (EcMap self, void* key, void* val)
{
  EcMapNode node = ENTC_NEW(struct EcMapNode_s);

  node->key = key;
  node->val = val;
  node->map = self;

  eclist_push_back (self->list, node);

  return node;
}

//----------------------------------------------------------------------------------------

EcMapNode ecmap_find (EcMap self, void* key)
{
  EcListCursor cursor;

  eclist_cursor_init (self->list, &cursor, LIST_DIR_NEXT);

  while (eclist_cursor_next (&cursor))
  {
    EcMapNode node = eclist_data (cursor.node);

    //compare the stored name with the name we got
    if (self->onCompare (node->key, key) == 0)
    {
      return node;
    }
  }

  return NULL;
}

//----------------------------------------------------------------------------------------

void ecmap_destroy_node (EcMap self, EcMapNode* pnode)
{
  EcMapNode node = *pnode;
  
  if (self->onDestroy)
  {
    self->onDestroy (node->key, node->val);
  }
  
  ecmap_node_destroy (pnode);
}

//----------------------------------------------------------------------------------------

void ecmap_erase (EcMap self, EcMapNode node)
{
  EcListCursor cursor;

  eclist_cursor_init (self->list, &cursor, LIST_DIR_NEXT);

  while (eclist_cursor_next (&cursor))
  {
    EcMapNode h = eclist_data (cursor.node);

    if (h == node)
    {
      eclist_cursor_erase (self->list, &cursor);
      return;
    }
  }
}

//----------------------------------------------------------------------------------------

EcMapNode ecmap_extract (EcMap self, EcMapNode node)
{
  EcListCursor cursor;

  eclist_cursor_init (self->list, &cursor, LIST_DIR_NEXT);

  while (eclist_cursor_next (&cursor))
  {
    EcMapNode h = eclist_data (cursor.node);

    if (h == node)
    {
      return eclist_cursor_extract (self->list, &cursor);
    }
  }

  return NULL;
}

//----------------------------------------------------------------------------------------

unsigned long ecmap_size (EcMap self)
{
  return eclist_size (self->list);
}

//----------------------------------------------------------------------------------------

EcMap ecmap_clone (EcMap orig, fct_ecmap_onClone onCloneKey, fct_ecmap_onClone onCloneVal)
{
  EcMap self = ecmap_create (orig->onCompare, orig->onDestroy);

  EcListCursor cursor;
  eclist_cursor_init (orig->list, &cursor, LIST_DIR_NEXT);

  while (eclist_cursor_next (&cursor))
  {
    EcMapNode h = eclist_data (cursor.node);

    void* key = NULL;
    void* val = NULL;

    if (onCloneKey)
    {
      key = onCloneKey (h->key);
    }

    if (onCloneVal)
    {
      val = onCloneVal (h->val);
    }

    ecmap_insert (self, key, val);
  }

  return self;
}

//----------------------------------------------------------------------------------------

EcMapCursor* ecmap_cursor_create (EcMap self, int direction)
{
  EcMapCursor* cursor = ENTC_NEW(EcMapCursor);

  ecmap_cursor_init (self, cursor, direction);

  return cursor;
}

//----------------------------------------------------------------------------------------

void ecmap_cursor_destroy (EcMapCursor** pcursor)
{
  ENTC_DEL(pcursor, EcMapCursor);
}

//----------------------------------------------------------------------------------------

void ecmap_cursor_set (EcMapCursor* cursor)
{
  if (cursor->cursor.node)
  {
    cursor->node = eclist_data (cursor->cursor.node);
  }
  else
  {
    cursor->node = NULL;
  }
  
  cursor->position = cursor->cursor.position;
}

//----------------------------------------------------------------------------------------

void ecmap_cursor_init (EcMap self, EcMapCursor* cursor, int direction)
{
  eclist_cursor_init (self->list, &(cursor->cursor), direction);

  ecmap_cursor_set (cursor);
}

//----------------------------------------------------------------------------------------

int ecmap_cursor_next (EcMapCursor* cursor)
{
  int res = eclist_cursor_next (&(cursor->cursor));

  ecmap_cursor_set (cursor);

  return res;
}

//----------------------------------------------------------------------------------------

int ecmap_cursor_prev (EcMapCursor* cursor)
{
  int res = eclist_cursor_prev (&(cursor->cursor));

  ecmap_cursor_set (cursor);

  return res;
}

//----------------------------------------------------------------------------------------

void ecmap_cursor_erase (EcMap self, EcMapCursor* cursor)
{
  eclist_cursor_erase(self->list, &(cursor->cursor));

  ecmap_cursor_set (cursor);
}

//----------------------------------------------------------------------------------------

EcMapNode ecmap_cursor_extract (EcMap self, EcMapCursor* cursor)
{
  EcMapNode node = eclist_cursor_extract(self->list, &(cursor->cursor));

  ecmap_cursor_set (cursor);

  return node;
}

//----------------------------------------------------------------------------------------
