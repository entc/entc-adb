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

#ifndef ENTC_TOOLS_ASYNCSV_H
#define ENTC_TOOLS_ASYNCSV_H 1

#include "../system/macros.h"
#include "../system/types.h"
#include "../utils/eclogger.h"

#include "system/ecsocket.h"
#include "system/ecevents.h"
#include "system/ecasyncio.h"

// **** worker context ****

typedef void (_STDCALL *ecasync_worker_rawdata_cb)(void* ptr, const unsigned char* buffer, ulong_t len);

__CPP_EXTERN______________________________________________________________________________START

__LIB_EXPORT EcAsyncContext ecasync_worker_create (EcSocket sock, ulong_t timeout, ecasync_worker_rawdata_cb, void*);

__CPP_EXTERN______________________________________________________________________________END

typedef int (_STDCALL *ecasync_worker_recv_cb)(void* ptr, const unsigned char* buffer, ulong_t len);

typedef ulong_t (_STDCALL *ecasync_worker_idle_cb)(void* ptr);

__CPP_EXTERN______________________________________________________________________________START

__LIB_EXPORT EcAsyncContext _STDCALL ecasync_strict_worker_create (EcSocket sock, ulong_t timeout, ecasync_worker_idle_cb, ecasync_worker_recv_cb, void*);

__CPP_EXTERN______________________________________________________________________________END

// **** accept context ****

typedef EcAsyncContext (_STDCALL *ecasync_accept_worker_cb)(void*, EcSocket sock);

__CPP_EXTERN______________________________________________________________________________START

__LIB_EXPORT EcAsyncContext ecasync_accept_create (const EcString host, ulong_t port, EcEventContext ec, EcAsync async, ecasync_accept_worker_cb cb, void*);

__CPP_EXTERN______________________________________________________________________________END

/*
// **** server ****

struct EcAsyncServ_s; typedef struct EcAsyncServ_s* EcAsyncServ;

__CPP_EXTERN______________________________________________________________________________START

__LIB_EXPORT EcAsyncServ ecaserv_create ();

__LIB_EXPORT void ecaserv_destroy (EcAsyncServ*);

__LIB_EXPORT int ecaserv_start (EcAsyncServ, const EcString host, ulong_t port);

__LIB_EXPORT void ecaserv_stop (EcAsyncServ);

// misc / debug

__LIB_EXPORT void ecaserv_run (EcAsyncServ);

__CPP_EXTERN______________________________________________________________________________END
 
 */

#endif