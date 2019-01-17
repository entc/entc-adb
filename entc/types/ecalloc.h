/*
 * Copyright (c) 2010-2015 "Alexander Kalkhof" [email:alex@kalkhof.org]
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

#ifndef ENTC_TYPES_ALLOC_H
#define ENTC_TYPES_ALLOC_H 1

#include "sys/entc_export.h"
#include "sys/entc_types.h"

typedef void*  (__STDCALL *ecnew_fct)(void*, uint32_t);

typedef void   (__STDCALL *ecdel_fct)(void*, void**, uint32_t);

typedef void   (__STDCALL *ecset_fct)(void*, void* addr, void* val);

typedef void*  (__STDCALL *ecget_fct)(void*, void* addr);

#pragma pack(push, 16)
struct EcAlloc_s
{
  
  void* ptr;
  
  ecnew_fct fnew;
  
  ecdel_fct fdel;
  
  ecset_fct fset;
  
  ecget_fct fget;
  
}; typedef struct EcAlloc_s* EcAlloc;
#pragma pack(pop)

//-------------------------------------------------------------------------------------------

__ENTC_LIBEX void* __STDCALL EC_NEW (void* ptr, uint32_t size);
__ENTC_LIBEX void  __STDCALL EC_DEL (void* ptr, void** pobj, uint32_t size);

static struct EcAlloc_s EC_ALLOC_S = {NULL, EC_NEW, EC_DEL};

static EcAlloc EC_ALLOC = &EC_ALLOC_S;

#define ECMM_NEW(dtype) alloc->fnew (alloc->ptr, sizeof(dtype))
#define ECMM_DEL(dptr, dtype) alloc->fdel (alloc->ptr, (void**)dptr, sizeof(dtype))

#endif
