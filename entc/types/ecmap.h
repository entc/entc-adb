/*
 * Copyright (c) 2010-2017 "Alexander Kalkhof" [email:alex@kalkhof.org]
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

#ifndef ENTC_TYPES_MAP_H
#define ENTC_TYPES_MAP_H 1

//=============================================================================

#include "ecerr.h"
#include "stc/entc_list.h"

//-----------------------------------------------------------------------------

struct EcMapNode_s; typedef struct EcMapNode_s* EcMapNode;

//-----------------------------------------------------------------------------

__LIBEX void* ecmap_node_value (EcMapNode);

__LIBEX void* ecmap_node_key (EcMapNode);

__LIBEX void* ecmap_node_extract (EcMapNode);

__LIBEX void ecmap_node_destroy (EcMapNode*);

//-----------------------------------------------------------------------------

struct EcMap_s; typedef struct EcMap_s* EcMap;

typedef int   (__STDCALL *fct_ecmap_cmp)      (const void* a, const void* b);
typedef void  (__STDCALL *fct_ecmap_destroy)  (void* key, void* val);

//-----------------------------------------------------------------------------

__LIBEX EcMap ecmap_create (fct_ecmap_cmp, fct_ecmap_destroy);

__LIBEX void ecmap_clear (EcMap);

__LIBEX void ecmap_destroy (EcMap*);

__LIBEX void ecmap_setcmp (EcMap, fct_ecmap_cmp);

__LIBEX EcMapNode ecmap_insert (EcMap, void* key, void* data);

__LIBEX EcMapNode ecmap_find (EcMap, void* key);

__LIBEX void ecmap_erase (EcMap, EcMapNode);          // removes the node, calls the onDestroy callback and releases the node

__LIBEX EcMapNode ecmap_extract (EcMap, EcMapNode);   // extracts the node from the container and returns it

__LIBEX void ecmap_destroy_node (EcMap, EcMapNode*);  // calls the onDestroy callback and releases the node

__LIBEX unsigned long ecmap_size (EcMap);

//-----------------------------------------------------------------------------

typedef void* (__STDCALL *fct_ecmap_onClone) (void* ptr);

__LIBEX EcMap ecmap_clone (EcMap, fct_ecmap_onClone onCloneKey, fct_ecmap_onClone onCloneVal);

//-----------------------------------------------------------------------------

#ifndef RB_ITER_MAX_HEIGHT
#define RB_ITER_MAX_HEIGHT 64 // Tallest allowable tree to iterate
#endif

typedef struct
{
  
  EcMapNode node;
  
  int direction;

  EntcListCursor cursor;
  
  int position;

} EcMapCursor;

//-----------------------------------------------------------------------------

#define LIST_DIR_NEXT 1
#define LIST_DIR_PREV 0

//-----------------------------------------------------------------------------

__LIBEX EcMapCursor* ecmap_cursor_create (EcMap, int direction);

__LIBEX void ecmap_cursor_destroy (EcMapCursor**);

__LIBEX void ecmap_cursor_init (EcMap, EcMapCursor*, int direction);

__LIBEX int ecmap_cursor_next (EcMapCursor*);

__LIBEX int ecmap_cursor_prev (EcMapCursor*);

__LIBEX void ecmap_cursor_erase (EcMap, EcMapCursor*);

__LIBEX EcMapNode ecmap_cursor_extract (EcMap, EcMapCursor*);

//-----------------------------------------------------------------------------

#endif
