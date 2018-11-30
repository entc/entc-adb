/*
 * Copyright (c) 2010-2018 "Alexander Kalkhof" [email:alex@kalkhof.org]
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

//=============================================================================

#include "system/ecdefs.h"
#include "types/ecstring.h"
#include "types/ecbuffer.h"

#include <sys/time.h>

//=============================================================================

struct EcStream_s; typedef struct EcStream_s* EcStream;

//-----------------------------------------------------------------------------

__LIBEX EcStream        ecstream_create (void);

__LIBEX void            ecstream_destroy (EcStream*);

__LIBEX void            ecstream_clear (EcStream);

__LIBEX const char*     ecstream_get (EcStream);

__LIBEX unsigned long   ecstream_size (EcStream);

//-----------------------------------------------------------------------------
// convert to other types

__LIBEX EcBuffer        ecstream_tobuf (EcStream*);

__LIBEX EcString        ecstream_tostr (EcStream*);

//-----------------------------------------------------------------------------
// append functions

__LIBEX void            ecstream_append_str (EcStream, const char*);

__LIBEX void            ecstream_append_buf (EcStream, const char*, unsigned long size);

__LIBEX void            ecstream_append_ecbuf (EcStream, const EcBuffer);

__LIBEX void            ecstream_append_fmt (EcStream, const char*, ...);

__LIBEX void            ecstream_append_c (EcStream, char);

__LIBEX void            ecstream_append_u (EcStream, unsigned long);

__LIBEX void            ecstream_append_u64 (EcStream, uint64_t);

__LIBEX void            ecstream_append_i (EcStream, long);

__LIBEX void            ecstream_append_i64 (EcStream, int64_t);

__LIBEX void            ecstream_append_time (EcStream, const time_t*);

__LIBEX void            ecstream_append_stream (EcStream, EcStream);

//=============================================================================

struct EcDevStream_s; typedef struct EcDevStream_s* EcDevStream;

typedef void (*stream_callback_fct)(void* ptr, const void* buffer, uint_t nbyte);

//-----------------------------------------------------------------------------

__LIBEX EcDevStream ecdevstream_new (uint_t size, stream_callback_fct, void*);

__LIBEX void ecdevstream_delete (EcDevStream*);

__LIBEX void ecdevstream_flush (EcDevStream);

__LIBEX void ecdevstream_append (EcDevStream, void*, uint_t size);

__LIBEX void ecdevstream_appends (EcDevStream, const EcString);

__LIBEX void ecdevstream_appendc (EcDevStream, char);

__LIBEX void ecdevstream_appendu (EcDevStream, uint_t);

__LIBEX void ecdevstream_appendfile (EcDevStream, const EcString filename);

//-----------------------------------------------------------------------------

#endif
