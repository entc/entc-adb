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

#ifndef ENTC_SYSTEM_SOCKET_H
#define ENTC_SYSTEM_SOCKET_H 1

#include "../system/macros.h"
#include "../system/types.h"

#include "../types/ecstring.h"
#include "../types/ecstream.h"
#include "../utils/eclogger.h"

#include "ecfile.h"
#include "ecevents.h"

struct EcSocket_s; typedef struct EcSocket_s* EcSocket;

#define ENTC_SOCKET_RETSTATE_ABORT -1
#define ENTC_SOCKET_RETSTATE_ERROR -2
#define ENTC_SOCKET_RETSTATE_TIMEOUT -3

#define ENTC_SOCKET_PROTOCOL_TCP 1
#define ENTC_SOCKET_PROTOCOL_UDP 2

struct EcDatagram_s; typedef struct EcDatagram_s* EcDatagram;

__ENTC_LIBEX 

__ENTC_LIBEX EcSocket ecsocket_new (EcEventContext, int protocol);
  
__ENTC_LIBEX void ecsocket_delete (EcSocket*);

__ENTC_LIBEX int ecsocket_connect (EcSocket, const EcString host, uint_t port);

__ENTC_LIBEX int ecsocket_listen (EcSocket, const EcString host, uint_t port);

// simple data methods

__ENTC_LIBEX EcSocket ecsocket_accept (EcSocket);

__ENTC_LIBEX int ecsocket_readBunch (EcSocket, void* buffer, int nbyte);
  
__ENTC_LIBEX int ecsocket_write (EcSocket, const void* buffer, int nbyte);

__ENTC_LIBEX int ecsocket_writeStream (EcSocket, EcStream);

__ENTC_LIBEX int ecsocket_writeFile (EcSocket, EcFileHandle);

// udp datagrams

__ENTC_LIBEX EcDatagram ecdatagram_create (EcSocket);

__ENTC_LIBEX void ecdatagram_destroy (EcDatagram*);

__ENTC_LIBEX size_t ecdatagram_read (EcDatagram);

__ENTC_LIBEX size_t ecdatagram_write (EcDatagram, size_t len);

__ENTC_LIBEX size_t ecdatagram_writeBuf (EcDatagram, EcBuffer buf, size_t len);

__ENTC_LIBEX EcBuffer ecdatagram_buffer (EcDatagram);

__ENTC_LIBEX const EcString ecdatagram_ident (EcDatagram);

// interuptable data methods

__ENTC_LIBEX EcSocket ecsocket_acceptIntr (EcSocket);

__ENTC_LIBEX int ecsocket_readIntr (EcSocket, void* buffer, int nbyte, int sec);

__ENTC_LIBEX int ecsocket_readIntrBunch (EcSocket, void* buffer, int nbyte, int sec);

// misc methods

__ENTC_LIBEX EcHandle ecsocket_getAcceptHandle (EcSocket);

__ENTC_LIBEX EcHandle ecsocket_getReadHandle (EcSocket);

__ENTC_LIBEX void ecsocket_resetHandle (EcHandle);

__ENTC_LIBEX const EcString ecsocket_address (EcSocket);
  
__ENTC_LIBEX

#endif

