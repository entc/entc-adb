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

#ifndef ENTC_TYPES_MAP_H
#define ENTC_TYPES_MAP_H 1

#include "../system/macros.h"
#include "../types/ecstring.h"

#include "eclist.h"

struct EcMap_s; typedef struct EcMap_s* EcMap;  
typedef struct EcListNode_s* EcMapNode;

typedef struct {
  
  EcListNode node;
  
  EcMap map;
  
} EcMapCursor;

__CPP_EXTERN______________________________________________________________________________START

__LIB_EXPORT EcMap ecmap_new();
  
__LIB_EXPORT void ecmap_delete(EcMap*);

__LIB_EXPORT void ecmap_clear(EcMap);

__LIB_EXPORT EcMapNode ecmap_append(EcMap, const EcString key, void* data);
  
__LIB_EXPORT EcMapNode ecmap_erase(EcMapNode);
  
__LIB_EXPORT EcMapNode ecmap_find(EcMap, const EcString key);
  
__LIB_EXPORT EcMapNode ecmap_first(const EcMap);

__LIB_EXPORT EcMapNode ecmap_end(const EcMap);
  
__LIB_EXPORT EcMapNode ecmap_next(const EcMapNode);
  
__LIB_EXPORT EcString ecmap_key(const EcMapNode);
  
__LIB_EXPORT void* ecmap_data(const EcMapNode);
  
__LIB_EXPORT uint_t ecmap_size(const EcMap);

// cursor

__LIB_EXPORT void ecmap_cursor (EcMap, EcMapCursor*);

__LIB_EXPORT int ecmap_cnext (EcMapCursor* c);

__CPP_EXTERN______________________________________________________________________________END

#endif
