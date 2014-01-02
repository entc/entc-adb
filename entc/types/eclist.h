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

#include "../system/macros.h"
#include "../system/types.h"

struct EcList_s; typedef struct EcList_s* EcList;

struct EcListNode_s; typedef struct EcListNode_s* EcListNode;

__CPP_EXTERN______________________________________________________________________________START

__LIB_EXPORT EcList eclist_new();
  
__LIB_EXPORT void eclist_delete(EcList*);
  
__LIB_EXPORT EcListNode eclist_append(EcList, void* data);
  
__LIB_EXPORT void eclist_set(EcListNode, void* data);

__LIB_EXPORT EcListNode eclist_erase(EcListNode);
  
__LIB_EXPORT EcListNode eclist_first(const EcList);

__LIB_EXPORT EcListNode eclist_last(const EcList);

__LIB_EXPORT EcListNode eclist_end(const EcList);

__LIB_EXPORT EcListNode eclist_next(const EcListNode);

__LIB_EXPORT EcListNode eclist_back(const EcListNode);
  
__LIB_EXPORT void* eclist_data(const EcListNode);
  
__LIB_EXPORT void eclist_clear(EcList);
  
__LIB_EXPORT uint_t eclist_size(const EcList);
  
__LIB_EXPORT void eclist_remove(EcList, void*);
  
__CPP_EXTERN______________________________________________________________________________END

#endif
