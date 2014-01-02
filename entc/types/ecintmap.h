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

#ifndef ENTC_TYPES_INTMAP_H
#define ENTC_TYPES_INTMAP_H 1

/* include external macro for win32 */
#include "../system/macros.h"
#include "../system/types.h"

#include "eclist.h"

struct EcIntMap_s;


typedef struct EcIntMap_s* EcIntMap;

typedef struct EcListNode_s* EcIntMapNode;

__CPP_EXTERN______________________________________________________________________________START

__LIB_EXPORT EcIntMap ecintmap_new();
  
__LIB_EXPORT void ecintmap_delete(EcIntMap*);
  
__LIB_EXPORT void ecintmap_clear(EcIntMap);
  
__LIB_EXPORT void ecintmap_append(EcIntMap, uint_t key, void* data);
  
__LIB_EXPORT EcIntMapNode ecintmap_find(EcIntMap, uint_t key);
  
__LIB_EXPORT EcIntMapNode ecintmap_first(const EcIntMap);
  
__LIB_EXPORT EcIntMapNode ecintmap_next(const EcIntMapNode);
  
__LIB_EXPORT EcIntMapNode ecintmap_end(EcIntMap);
  
__LIB_EXPORT void* ecintmap_data(const EcIntMapNode);
  
  /* extra functions */
__LIB_EXPORT void ecintmap_orderAll(EcIntMap);
  
__CPP_EXTERN______________________________________________________________________________END 

#endif

