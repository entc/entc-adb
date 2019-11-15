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

#include "sys/entc_export.h"
#include "types/ecstring.h"
#include "types/ecbuffer.h"

//=============================================================================

struct EcStream_s; typedef struct EcStream_s* EcStream;

//-----------------------------------------------------------------------------

__ENTC_LIBEX EcStream        ecstream_create (void);

__ENTC_LIBEX void            ecstream_destroy (EcStream*);

__ENTC_LIBEX void            ecstream_clear (EcStream);

__ENTC_LIBEX const char*     ecstream_get (EcStream);

__ENTC_LIBEX unsigned long   ecstream_size (EcStream);

//-----------------------------------------------------------------------------
// convert to other types

__ENTC_LIBEX EcBuffer        ecstream_tobuf (EcStream*);

__ENTC_LIBEX EcString        ecstream_tostr (EcStream*);

//-----------------------------------------------------------------------------
// append functions

__ENTC_LIBEX void            ecstream_append_str (EcStream, const char*);

__ENTC_LIBEX void            ecstream_append_buf (EcStream, const char*, unsigned long size);

__ENTC_LIBEX void            ecstream_append_ecbuf (EcStream, const EcBuffer);

__ENTC_LIBEX void            ecstream_append_fmt (EcStream, const char*, ...);

__ENTC_LIBEX void            ecstream_append_c (EcStream, char);

__ENTC_LIBEX void            ecstream_append_u (EcStream, unsigned long);

__ENTC_LIBEX void            ecstream_append_u64 (EcStream, uint64_t);

__ENTC_LIBEX void            ecstream_append_i (EcStream, long);

__ENTC_LIBEX void            ecstream_append_i64 (EcStream, int64_t);

__ENTC_LIBEX void            ecstream_append_time (EcStream, const time_t*);

__ENTC_LIBEX void            ecstream_append_stream (EcStream, EcStream);

//=============================================================================

struct EcDevStream_s; typedef struct EcDevStream_s* EcDevStream;

typedef void (*stream_callback_fct)(void* ptr, const void* buffer, uint_t nbyte);

//-----------------------------------------------------------------------------

__ENTC_LIBEX EcDevStream ecdevstream_new (uint_t size, stream_callback_fct, void*);

__ENTC_LIBEX void ecdevstream_delete (EcDevStream*);

__ENTC_LIBEX void ecdevstream_flush (EcDevStream);

__ENTC_LIBEX void ecdevstream_append (EcDevStream, void*, uint_t size);

__ENTC_LIBEX void ecdevstream_appends (EcDevStream, const EcString);

__ENTC_LIBEX void ecdevstream_appendc (EcDevStream, char);

__ENTC_LIBEX void ecdevstream_appendu (EcDevStream, uint_t);

__ENTC_LIBEX void ecdevstream_appendfile (EcDevStream, const EcString filename);

//-----------------------------------------------------------------------------

#endif
