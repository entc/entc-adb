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

#ifndef ENTC_TYPES_ARRAY_H
#define ENTC_TYPES_ARRAY_H 1

#include "../system/macros.h"
#include "../system/types.h"

struct EcArray_s; typedef struct EcArray_s* EcArray;

struct EcArrayNode_s; typedef struct EcArrayNode_s* EcArrayNode;

__CPP_EXTERN______________________________________________________________________________START
  
__LIB_EXPORT EcArray ecarray_new();
  
__LIB_EXPORT void ecarray_delete(EcArray*);
  
__LIB_EXPORT void ecarray_clear(EcArray);

__LIB_EXPORT void ecarray_resize(EcArray, uint_t size);

__LIB_EXPORT void ecarray_set(EcArray, uint_t index, void* data);

__LIB_EXPORT void* ecarray_get(EcArray, uint_t index);
  
__CPP_EXTERN______________________________________________________________________________END

#endif
