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

#ifndef ENTC_UTILS_READBUFFER_H
#define ENTC_UTILS_READBUFFER_H 1

#include "../system/macros.h"
#include "../system/types.h"
#include "../system/ecfile.h"

#include "../types/ecstring.h"

struct EcReadBuffer_s; typedef struct EcReadBuffer_s* EcReadBuffer;

__CPP_EXTERN______________________________________________________________________________START
    
__LIB_EXPORT EcReadBuffer ecreadbuffer_create (EcFileHandle, int close);

__LIB_EXPORT void ecreadbuffer_destroy (EcReadBuffer*);

__LIB_EXPORT int ecreadbuffer_getnext (EcReadBuffer, char* character);

  /* ensure to read size amount of characters into the buffer */
  /* return the real amount, could be < size if end of file reached */
__LIB_EXPORT uint_t ecreadbuffer_get ( EcReadBuffer, uint_t size );

__LIB_EXPORT const EcString ecreadbuffer_buffer ( EcReadBuffer );

__CPP_EXTERN______________________________________________________________________________END

#endif
