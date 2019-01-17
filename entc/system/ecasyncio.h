/*
 * Copyright (c) 2010-2015 "Alexander Kalkhof" [email:entc@kalkhof.org]
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

#ifndef ENTC_TOOLS_ASYNCIO_H
#define ENTC_TOOLS_ASYNCIO_H 1

#include "sys/entc_export.h"
#include "sys/entc_types.h"
#include "system/ecevents.h"

// context

typedef EcHandle __ENTC_LIBEX *ecasync_context_handle_cb)(void* ptr);
typedef int __ENTC_LIBEX *ecasync_context_run_cb)(void* ptr);
typedef void __ENTC_LIBEX *ecasync_context_destroy_cb)(void** ptr);
typedef void* __ENTC_LIBEX *ecasync_context_create_cb)(void);

typedef struct
{

  ecasync_context_destroy_cb destroy;

  ecasync_context_handle_cb handle;
  
  ecasync_context_run_cb run;
  
  ece_list_sort_out_fct timeout;
  
} EcAsyncContextCallbacks;

struct EcAsyncContext_s; typedef struct EcAsyncContext_s* EcAsyncContext;

__ENTC_LIBEX EcAsyncContext ecasync_context_create (const EcAsyncContextCallbacks*, void* ptr);

__ENTC_LIBEX void ecasync_context_destroy (EcAsyncContext*);



// manager

struct EcAsync_s; typedef struct EcAsync_s* EcAsync;

__ENTC_LIBEX EcAsync ecasync_create (int threads);

__ENTC_LIBEX void ecasync_destroy (EcAsync*);

__ENTC_LIBEX void ecasync_addSingle (EcAsync, EcAsyncContext*);

__ENTC_LIBEX void ecasync_addToAll (EcAsync, EcAsyncContext*);



#endif
