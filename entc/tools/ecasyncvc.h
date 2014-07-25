/*
 * Copyright (c) 2010-2013 "Alexander Kalkhof" [email:entc@kalkhof.org]
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

#ifndef ENTC_TOOLS_ASYNCSV_H
#define ENTC_TOOLS_ASYNCSV_H 1

#include "../system/macros.h"
#include "../system/types.h"
#include "../utils/eclogger.h"

#include "system/ecevents.h"

struct EcAsyncSvc_s; typedef struct EcAsyncSvc_s* EcAsyncSvc;

typedef int (*async_context_fct)(void* ptr, EcAsyncSvc);

typedef struct
{
  
  EcHandle handle;
  
  async_context_fct fct;
  
  void* ptr;
  
  
} EcAsyncContext;


__CPP_EXTERN______________________________________________________________________________START

__LIB_EXPORT EcAsyncSvc ecasyncsvc_create (void);

__LIB_EXPORT void ecasyncsvc_destroy (EcAsyncSvc*);

__LIB_EXPORT void ecasyncsvc_start (EcAsyncSvc);

__LIB_EXPORT void ecasyncsvc_add (EcAsyncSvc, EcAsyncContext*);

__CPP_EXTERN______________________________________________________________________________END

#endif