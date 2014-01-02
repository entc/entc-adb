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

#ifndef ENTC_TYPES_PTRMAP_H
#define ENTC_TYPES_PTRMAP_H 1

#include "../system/macros.h"

#include "../types/ecstring.h"
#include "../types/eclist.h"

struct EcPtrMap_s;

typedef struct EcPtrMap_s* EcPtrMap;

typedef struct EcListNode_s* EcPtrMapNode;

__CPP_EXTERN______________________________________________________________________________START
  
__LIB_EXPORT EcPtrMap ecptrmap_new();
  
__LIB_EXPORT void ecptrmap_delete(EcPtrMap*);
  
__LIB_EXPORT void ecptrmap_clear(EcPtrMap);
  
__LIB_EXPORT void ecptrmap_append(EcPtrMap, void* key, void* data);
  
__LIB_EXPORT EcPtrMapNode ecptrmap_erase(EcPtrMapNode);
  
__LIB_EXPORT EcPtrMapNode ecptrmap_find(EcPtrMap, void* key);
  
__LIB_EXPORT void* ecptrmap_finddata(EcPtrMap, void* key);
  
__LIB_EXPORT EcPtrMapNode ecptrmap_first(const EcPtrMap);
  
__LIB_EXPORT EcPtrMapNode ecptrmap_next(const EcPtrMapNode);
  
__LIB_EXPORT EcPtrMapNode ecptrmap_end(EcPtrMap);
  
__LIB_EXPORT void* ecptrmap_key(const EcPtrMapNode);
  
__LIB_EXPORT void* ecptrmap_data(const EcPtrMapNode);
  
__LIB_EXPORT void ecptrmap_reset(const EcPtrMapNode, void* data);
  
__CPP_EXTERN______________________________________________________________________________END

#endif

