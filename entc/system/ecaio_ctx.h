/*
 * Copyright (c) 2010-2017 "Alexander Kalkhof" [email:entc@kalkhof.org]
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

#ifndef ENTC_SYSTEM_AIO_CTX_H
#define ENTC_SYSTEM_AIO_CTX_H 1

//-----------------------------------------------------------------------------

#include "types/ecerr.h"

//-----------------------------------------------------------------------------

#define ENTC_AIO_CODE_UNKNOWN        0
#define ENTC_AIO_CODE_CONTINUE      10
#define ENTC_AIO_CODE_DONE          11
#define ENTC_AIO_CODE_ABORTALL      12

//=============================================================================

struct EcAioContext_s; typedef struct EcAioContext_s* EcAioContext;

typedef int  (__STDCALL *fct_ecaio_context_process)  (void* ptr, EcAioContext, unsigned long, unsigned long);
typedef void (__STDCALL *fct_ecaio_context_destroy)  (void* ptr);
typedef int  (__STDCALL *fct_ecaio_context_onRead)   (void* ptr, void* handle, const char* buffer, unsigned long);
typedef int  (__STDCALL *fct_ecaio_context_onInit)   (void* ptr, EcErr);
typedef int  (__STDCALL *fct_ecaio_context_onNotify) (void* ptr, int action);

//-----------------------------------------------------------------------------

__LIBEX EcAioContext ecaio_context_create (void);

__LIBEX void ecaio_context_destroy (EcAioContext*);

__LIBEX void ecaio_context_setCallbacks (EcAioContext, void* ptr, fct_ecaio_context_process, fct_ecaio_context_destroy);

__LIBEX int ecaio_context_process (EcAioContext, unsigned long val1, unsigned long val2);

// only for windows
__LIBEX void* ecaio_context_getOverlapped (EcAioContext);

__LIBEX void ecaio_context_appendOverlappedOffset (EcAioContext, int offset);

__LIBEX int ecaio_context_continue (void* ovl, int repeat, unsigned long bytes);

// only for linux
__LIBEX void ecaio_context_setHandle (EcAioContext, void* handle);

__LIBEX void* ecaio_context_getHandle (EcAioContext);

//=============================================================================

struct EcAioRefCtx_s; typedef struct EcAioRefCtx_s* EcAioRefCtx;

//-----------------------------------------------------------------------------

__LIBEX EcAioRefCtx ecaio_refctx_create ();

__LIBEX EcAioRefCtx ecaio_refctx_clone (EcAioRefCtx);

__LIBEX void ecaio_refctx_decrease (EcAioRefCtx*);

__LIBEX void ecaio_refctx_setCallbacks (EcAioRefCtx, void* ptr, fct_ecaio_context_process process, fct_ecaio_context_destroy destroy);

__LIBEX int ecaio_refctx_process (EcAioRefCtx, EcAioContext, unsigned long val1, unsigned long val2);

//-----------------------------------------------------------------------------

#if defined __APPLE__

#define __BSD_KEVENT

#elif __linux__

#define __LINUX_EPOLL

#elif defined __WIN_OS

#define __MS_IOCP

#endif

//-----------------------------------------------------------------------------

#endif
