/*
 * Copyright (c) 2010-2013 "Alexander Kalkhof" [email:alex@kalkhof.org]
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

#include "types/ecalloc.h"

struct EcList_s; typedef struct EcList_s* EcList;

struct EcListNode_s; typedef struct EcListNode_s* EcListNode;

typedef struct {
  
  EcListNode node;
  
  EcList list;
  
  void* value;
  
} EcListCursor;

__CPP_EXTERN______________________________________________________________________________START

__LIB_EXPORT EcList eclist_create (void);
  
__LIB_EXPORT void eclist_free (EcList*);

__LIB_EXPORT EcList eclist_create_ex (EcAlloc);

__LIB_EXPORT void eclist_free_ex (EcAlloc, EcList*);
  
__LIB_EXPORT EcListNode eclist_append (EcList, void* data);

__LIB_EXPORT EcListNode eclist_append_ex (EcAlloc, EcList, void* data);
  
__LIB_EXPORT void eclist_set(EcListNode, void* data);

__LIB_EXPORT EcListNode eclist_erase (EcAlloc, EcList, EcListNode);
  
__LIB_EXPORT EcListNode eclist_first(const EcList);

__LIB_EXPORT EcListNode eclist_last(const EcList);

__LIB_EXPORT EcListNode eclist_end(const EcList);

__LIB_EXPORT EcListNode eclist_next(const EcListNode);

__LIB_EXPORT EcListNode eclist_back(const EcListNode);
  
__LIB_EXPORT void* eclist_data(const EcListNode);
  
__LIB_EXPORT void eclist_clear(EcList);
  
__LIB_EXPORT uint_t eclist_size(const EcList);
  
__LIB_EXPORT void eclist_remove (EcAlloc alloc, EcList, void*);
  
// cursor

__LIB_EXPORT void eclist_cursor (EcList, EcListCursor*);

__LIB_EXPORT int eclist_cnext (EcListCursor* c);

__LIB_EXPORT void eclist_cerase (EcAlloc alloc, EcListCursor* c);

__CPP_EXTERN______________________________________________________________________________END

#endif
