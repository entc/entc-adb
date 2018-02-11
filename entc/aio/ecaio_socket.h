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

#ifndef ENTC_SYSTEM_AIO_SOCKET_H
#define ENTC_SYSTEM_AIO_SOCKET_H 1

//-----------------------------------------------------------------------------

#include "types/ecerr.h"
#include "aio/ecaio.h"

//=============================================================================

struct EcAcceptSocket_s; typedef struct EcAcceptSocket_s* EcAcceptSocket;

//-----------------------------------------------------------------------------

__LIBEX EcAcceptSocket ecacceptsocket_create ();

__LIBEX void ecacceptsocket_destroy (EcAcceptSocket*);

__LIBEX int ecacceptsocket_listen (EcAcceptSocket, const char* host, int port, EcErr);

__LIBEX void* ecacceptsocket_socket (EcAcceptSocket);

//=============================================================================

struct EcRefCountedSocket_s; typedef struct EcRefCountedSocket_s* EcRefCountedSocket;

//-----------------------------------------------------------------------------

__LIBEX EcRefCountedSocket ecrefsocket_create (void*);

__LIBEX EcRefCountedSocket ecrefsocket_clone (EcRefCountedSocket);

__LIBEX void ecrefsocket_decrease (EcRefCountedSocket*);

__LIBEX void* ecrefsocket_socket (EcRefCountedSocket);

//=============================================================================

struct EcAioSocketReader_s; typedef struct EcAioSocketReader_s* EcAioSocketReader;

//-----------------------------------------------------------------------------

__LIBEX EcAioSocketReader ecaio_socketreader_create (void* handle);

__LIBEX void ecaio_socketreader_setCallback (EcAioSocketReader, void*, fct_ecaio_context_onRead, fct_ecaio_context_destroy);

__LIBEX int ecaio_socketreader_assign (EcAioSocketReader*, EcAio aio, EcErr err);

//=============================================================================

struct EcAioSocketWriter_s; typedef struct EcAioSocketWriter_s* EcAioSocketWriter;

//-----------------------------------------------------------------------------

__LIBEX EcAioSocketWriter ecaio_socketwriter_create (EcRefCountedSocket);

__LIBEX int ecaio_socketwriter_assign (EcAioSocketWriter*, EcErr err);

__LIBEX void ecaio_socketwriter_setBufferCP (EcAioSocketWriter, const char* buffer, unsigned long size);

__LIBEX void ecaio_socketwriter_setBufferBT (EcAioSocketWriter, EcBuffer*);

//=============================================================================

struct EcAioSocketAccept_s; typedef struct EcAioSocketAccept_s* EcAioSocketAccept;

typedef int (__STDCALL *fct_ecaio_socket_accept) (void* ptr, void* socket, const char* remoteAddress);

//-----------------------------------------------------------------------------

__LIBEX EcAioSocketAccept ecaio_socketaccept_create (void* socket);

__LIBEX void ecaio_socketaccept_setCallback (EcAioSocketAccept, void*, fct_ecaio_socket_accept);

__LIBEX int ecaio_socketaccept_assign (EcAioSocketAccept*, EcAio aio, EcErr err);

//-----------------------------------------------------------------------------

#endif
