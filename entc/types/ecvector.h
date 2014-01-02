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

#ifndef ENTC_TYPES_VECTOR_H
#define ENTC_TYPES_VECTOR_H 1

#include "../system/macros.h"
#include "../system/types.h"

#include "eclist.h"

struct EcVector_s; typedef struct EcVector_s* EcVector;
typedef struct EcListNode_s* EcVectorNode;

__CPP_EXTERN______________________________________________________________________________START
  
__LIB_EXPORT EcVector ecvector_new();
  
__LIB_EXPORT void ecvector_delete(EcVector*);
  
__LIB_EXPORT EcVectorNode ecvector_append(EcVector, void* data);
  
__LIB_EXPORT uint_t ecvector_add(EcVector, void* data);

__LIB_EXPORT EcVectorNode ecvector_find(EcVector, uint_t index);

__LIB_EXPORT void* ecvector_get(EcVector, uint_t index);
  
__LIB_EXPORT EcVectorNode ecvector_erase(EcVectorNode);
  
__LIB_EXPORT EcVectorNode ecvector_first(const EcVector);
  
__LIB_EXPORT EcVectorNode ecvector_last(const EcVector);
  
__LIB_EXPORT EcVectorNode ecvector_end(const EcVector);
  
__LIB_EXPORT EcVectorNode ecvector_next(const EcVectorNode);
  
__LIB_EXPORT EcVectorNode ecvector_back(const EcVectorNode);
  
__LIB_EXPORT EcVectorNode ecvector_at(const EcVector, uint_t index);
  
__LIB_EXPORT void* ecvector_data(const EcVectorNode);
  
__LIB_EXPORT uint_t ecvector_index(const EcVectorNode);
  
__LIB_EXPORT void ecvector_clear(EcVector);
  
__LIB_EXPORT uint_t ecvector_size(EcVector);
  
__CPP_EXTERN______________________________________________________________________________END

#endif
