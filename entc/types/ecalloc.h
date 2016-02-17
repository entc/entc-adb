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

#include "system/macros.h"
#include "system/types.h"

#ifndef ENTC_TYPES_ALLOC_H
#define ENTC_TYPES_ALLOC_H 1

typedef void*  (_STDCALL *ecnew_fct)(void*, uint32_t);

typedef void   (_STDCALL *ecdel_fct)(void*, void**, uint32_t);

struct EcAlloc_s
{
  
  void* ptr;
  
  ecnew_fct fnew;
  
  ecdel_fct fdel;
  
}; typedef struct EcAlloc_s* EcAlloc;

//-------------------------------------------------------------------------------------------

static void* _STDCALL EC_NEW (void* ptr, uint32_t size)
{
  return ENTC_MALLOC(size);
}

//-------------------------------------------------------------------------------------------

static void _STDCALL EC_DEL (void* ptr, void** pobj, uint32_t size)
{
  // clear all data
  memset (*pobj, 0, size);
  
  // free
  ENTC_FREE(*pobj);
  
  // return
  *pobj = NULL;
}

//-------------------------------------------------------------------------------------------

static struct EcAlloc_s EC_ALLOC_S = {NULL, EC_NEW, EC_DEL};

static EcAlloc EC_ALLOC = &EC_ALLOC_S;

#define ECMM_NEW(dtype) alloc->fnew (alloc->ptr, sizeof(dtype))
#define ECMM_DEL(dptr, dtype) alloc->fdel (alloc->ptr, (void**)dptr, sizeof(dtype))

#endif
