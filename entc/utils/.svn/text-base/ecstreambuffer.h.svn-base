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

#ifndef ENTC_UTILS_STREAMBUFFER_H
#define ENTC_UTILS_STREAMBUFFER_H 1

#include "../system/macros.h"
#include "../system/types.h"

#include "../types/ecstring.h"
#include "../types/ecstream.h"

#include "../utils/ecobserver.h"
#include "../utils/eclogger.h"
#include "../system/ecsocket.h"

struct EcStreamBuffer_s; typedef struct EcStreamBuffer_s* EcStreamBuffer;

__CPP_EXTERN______________________________________________________________________________START

__LIB_EXPORT EcStreamBuffer ecstreambuffer_new(EcLogger logger, EcSocket);

__LIB_EXPORT void ecstreambuffer_delete(EcStreamBuffer*);

__LIB_EXPORT int ecstreambuffer_next(EcStreamBuffer, int* error);

__LIB_EXPORT char ecstreambuffer_get(EcStreamBuffer);

__LIB_EXPORT const char* ecstreambuffer_buffer(EcStreamBuffer);

__LIB_EXPORT uint_t ecstreambuffer_filled(EcStreamBuffer, int* error);

__LIB_EXPORT uint_t ecstreambuffer_fill(EcStreamBuffer, int* error);
  
__LIB_EXPORT int ecstreambuffer_readln(EcStreamBuffer, EcStream stream, int* error);

__LIB_EXPORT void ecstreambuffer_read(EcStreamBuffer, EcStream stream, int* error);

__CPP_EXTERN______________________________________________________________________________END

#endif
