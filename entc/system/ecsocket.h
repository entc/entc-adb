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

__CPP_EXTERN______________________________________________________________________________START 

__LIB_EXPORT EcSocket ecsocket_new (EcEventContext, EcLogger);
  
__LIB_EXPORT void ecsocket_delete (EcSocket*);

__LIB_EXPORT int ecsocket_connect (EcSocket, const EcString host, uint_t port);

__LIB_EXPORT int ecsocket_listen (EcSocket, const EcString host, uint_t port);

__LIB_EXPORT EcSocket ecsocket_accept (EcSocket);

__LIB_EXPORT int ecsocket_read (EcSocket, void* buffer, int nbyte);
  
__LIB_EXPORT EcHandle ecsocket_getAcceptHandle (EcSocket);

__LIB_EXPORT EcHandle ecsocket_getReadHandle (EcSocket);

__LIB_EXPORT void ecsocket_resetHandle (EcHandle);

__LIB_EXPORT void ecsocket_closeHandle (EcHandle);

__LIB_EXPORT int ecsocket_write (EcSocket, const void* buffer, int nbyte);

// smart methods

__LIB_EXPORT int ecsocket_readAll (EcSocket, void* buffer, int nbyte, int sec);

__LIB_EXPORT int ecsocket_readBunch (EcSocket, void* buffer, int nbyte, int sec);

__LIB_EXPORT int ecsocket_writeStream (EcSocket, EcStream);

__LIB_EXPORT int ecsocket_writeFile (EcSocket, EcFileHandle);

__LIB_EXPORT const EcString ecsocket_address (EcSocket);
  
__CPP_EXTERN______________________________________________________________________________END

#endif

