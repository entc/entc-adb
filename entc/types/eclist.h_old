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

#ifndef ENTC_TYPES_LIST_H
#define ENTC_TYPES_LIST_H 1

//=============================================================================

#include "sys/entc_export.h"
#include "types/ecstring.h"

//-----------------------------------------------------------------------------

struct EcList_s; typedef struct EcList_s* EcList;

struct EcListNode_s; typedef struct EcListNode_s* EcListNode;

typedef int (__STDCALL *fct_eclist_onDestroy) (void* ptr);

//-----------------------------------------------------------------------------

__ENTC_LIBEX EcList eclist_create (fct_eclist_onDestroy);

__ENTC_LIBEX void eclist_destroy (EcList*);

__ENTC_LIBEX void eclist_clear (EcList);

__ENTC_LIBEX EcListNode eclist_push_back (EcList, void* data);

__ENTC_LIBEX EcListNode eclist_push_front (EcList, void* data);

__ENTC_LIBEX void* eclist_pop_front (EcList);

__ENTC_LIBEX void* eclist_pop_back (EcList);

__ENTC_LIBEX unsigned long eclist_size (EcList);

__ENTC_LIBEX int eclist_empty (EcList);

__ENTC_LIBEX int eclist_hasContent (EcList);

__ENTC_LIBEX void eclist_replace (EcList, EcListNode, void* data);

__ENTC_LIBEX void* eclist_data (EcListNode);

__ENTC_LIBEX void* eclist_extract (EcList, EcListNode);

__ENTC_LIBEX void eclist_erase (EcList, EcListNode);

__ENTC_LIBEX EcListNode eclist_next (EcListNode);

__ENTC_LIBEX EcListNode eclist_begin (EcList);

//-----------------------------------------------------------------------------

__ENTC_LIBEX EcList eclist_slice (EcList, EcListNode nodeFrom, EcListNode nodeTo);

__ENTC_LIBEX void eclist_swap (EcListNode, EcListNode);

//-----------------------------------------------------------------------------

typedef int (__STDCALL *fct_eclist_onCompare) (void* ptr1, void* ptr2);

__ENTC_LIBEX void eclist_sort (EcList, fct_eclist_onCompare);

//-----------------------------------------------------------------------------

typedef void* (__STDCALL *fct_eclist_onClone) (void* ptr);

__ENTC_LIBEX EcList eclist_clone (EcList, fct_eclist_onClone);

//-----------------------------------------------------------------------------

typedef struct
{
  
  EcListNode node;
  
  int position;
  
  int direction;
  
} EcListCursor;

//-----------------------------------------------------------------------------

#define LIST_DIR_NEXT 1
#define LIST_DIR_PREV 0

//-----------------------------------------------------------------------------

__ENTC_LIBEX EcListCursor* eclist_cursor_create (EcList, int direction);

__ENTC_LIBEX void eclist_cursor_destroy (EcListCursor**);

__ENTC_LIBEX void eclist_cursor_init (EcList, EcListCursor*, int direction);

__ENTC_LIBEX int eclist_cursor_next (EcListCursor*);

__ENTC_LIBEX int eclist_cursor_prev (EcListCursor*);

__ENTC_LIBEX void eclist_cursor_erase (EcList, EcListCursor*);

__ENTC_LIBEX void* eclist_cursor_extract (EcList, EcListCursor*);

__ENTC_LIBEX void eclist_insert_slice (EcList, EcListCursor*, EcList* pslice);

//-----------------------------------------------------------------------------

#endif
