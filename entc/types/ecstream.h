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

#ifndef ENTC_TYPES_STREAM_H
#define ENTC_TYPES_STREAM_H 1

#include "../system/macros.h"
#include "../system/types.h"

#include "ecstring.h"
#include "ecbuffer.h"

typedef void (*stream_callback_fct)(void* ptr, const void* buffer, uint_t nbyte);

struct EcStream_s; typedef struct EcStream_s* EcStream;

struct EcDevStream_s; typedef struct EcDevStream_s* EcDevStream;

__CPP_EXTERN______________________________________________________________________________START
    
__LIB_EXPORT EcStream ecstream_new (void);

__LIB_EXPORT void ecstream_delete (EcStream*);
  
__LIB_EXPORT void ecstream_clear( EcStream );
  
__LIB_EXPORT const EcString ecstream_buffer( EcStream );
  
__LIB_EXPORT void ecstream_appendd(EcStream, const EcString source, uint_t size);

__LIB_EXPORT void ecstream_append( EcStream, const EcString );

__LIB_EXPORT void ecstream_appendc( EcStream, char );

__LIB_EXPORT void ecstream_appendu( EcStream, uint_t );
  
__LIB_EXPORT uint_t ecstream_size( EcStream );

__LIB_EXPORT EcBuffer ecstream_trans (EcStream*);

//------ dev stream ----

__LIB_EXPORT EcDevStream ecdevstream_new (uint_t size, stream_callback_fct, void*);

__LIB_EXPORT void ecdevstream_delete (EcDevStream*);

__LIB_EXPORT void ecdevstream_flush (EcDevStream);

__LIB_EXPORT void ecdevstream_append (EcDevStream, void*, uint_t size);

__LIB_EXPORT void ecdevstream_appends (EcDevStream, const EcString);

__LIB_EXPORT void ecdevstream_appendc (EcDevStream, char);

__LIB_EXPORT void ecdevstream_appendu (EcDevStream, uint_t);

__LIB_EXPORT void ecdevstream_appendfile (EcDevStream, const EcString filename);

__CPP_EXTERN______________________________________________________________________________END

#endif
