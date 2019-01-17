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

typedef void __ENTC_LIBEX *ecasync_worker_rawdata_cb)(void* ptr, const unsigned char* buffer, ulong_t len);

typedef void __ENTC_LIBEX *ecasync_worker_destroy_cb)(void** ptr);

__ENTC_LIBEX

__ENTC_LIBEX EcAsyncContext ecasync_worker_create (EcSocket sock, ulong_t timeout, ecasync_worker_rawdata_cb, void*);

__ENTC_LIBEX

typedef int __ENTC_LIBEX *ecasync_worker_recv_cb)(void* ptr, const unsigned char* buffer, ulong_t len);

typedef ulong_t __ENTC_LIBEX *ecasync_worker_idle_cb)(void* ptr);

__ENTC_LIBEX

__ENTC_LIBEX EcAsyncContext _STDCALL ecasync_strict_worker_create (EcSocket sock, ulong_t timeout, ecasync_worker_idle_cb, ecasync_worker_recv_cb, void*);

__ENTC_LIBEX

// **** accept context ****

typedef EcAsyncContext __ENTC_LIBEX *ecasync_accept_worker_cb)(void*, EcSocket sock);

__ENTC_LIBEX

__ENTC_LIBEX EcAsyncContext ecasync_accept_create (const EcString host, ulong_t port, EcEventContext ec, EcAsync async, ecasync_accept_worker_cb cb, void*);

__ENTC_LIBEX

// **** udp worker and context ****

struct EcAsyncUdpContext_s; typedef struct EcAsyncUdpContext_s* EcAsyncUdpContext;

typedef EcAsyncUdpContext __ENTC_LIBEX *ecasync_dispatcher_cb)(void*);

typedef int __ENTC_LIBEX *ecasync_worker_udp_cb)(void* ptr, EcAsyncUdpContext, EcDatagram, ulong_t len);

struct EcAsynUdpDispatcher_s; typedef struct EcAsynUdpDispatcher_s* EcAsynUdpDispatcher;

__ENTC_LIBEX

// special context only for UDP protocol and UDP dispatcher
__ENTC_LIBEX EcAsyncUdpContext ecasync_udpcontext_create (ulong_t timeout, ecasync_worker_udp_cb, ecasync_worker_destroy_cb, void*);

// create a UDP dispatcher, whcih must be converted to a context later
// but can be kept as reference to call special methods of this object
__ENTC_LIBEX EcAsynUdpDispatcher ecasync_udpdisp_create (const EcString host, ulong_t port, EcEventContext ec, ecasync_dispatcher_cb cb, void*);

// convert dispatcher to default async context
__ENTC_LIBEX EcAsyncContext ecasync_udpdisp_context (EcAsynUdpDispatcher*);

// **** dispatcher methods ****

__ENTC_LIBEX void ecasync_udpdisp_broadcast (EcAsynUdpDispatcher, EcBuffer, size_t len, EcAsyncUdpContext ctx);

__ENTC_LIBEX

#endif