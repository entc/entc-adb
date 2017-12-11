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

#ifndef ENTC_TYPES_BUFFER_H
#define ENTC_TYPES_BUFFER_H 1

#include "system/macros.h"
#include "system/types.h"

#include "types/ecstring.h"
#include "types/ecerr.h"

#pragma pack(push, 16)
typedef struct { unsigned char* buffer; unsigned long size; } EcBuffer_s, * EcBuffer;
#pragma pack(pop)

__CPP_EXTERN______________________________________________________________________________START
  
// constructor and destructors

__LIB_EXPORT EcBuffer ecbuf_create (uint_t size);
  
__LIB_EXPORT EcBuffer ecbuf_create_filled (uint_t size, char fillupwith);

__LIB_EXPORT EcBuffer ecbuf_create_str (EcString* s);

__LIB_EXPORT EcBuffer ecbuf_create_uuid ();

__LIB_EXPORT EcBuffer ecbuf_create_fromBuffer (const unsigned char*, uint_t size);

__LIB_EXPORT EcBuffer ecbuf_create_fromStr (const EcString);

__LIB_EXPORT void ecbuf_destroy (EcBuffer*);

// manipulators

__LIB_EXPORT void ecbuf_setTerm (EcBuffer, uint_t size);

__LIB_EXPORT void ecbuf_fill (EcBuffer, uint_t size, char fillupwith);

__LIB_EXPORT void ecbuf_random (EcBuffer, uint_t size);
  
__LIB_EXPORT void ecbuf_format (EcBuffer, uint_t size, const char* format, ...);

__LIB_EXPORT void ecbuf_resize (EcBuffer, uint_t size);

__LIB_EXPORT ulong_t ecbuf_encode_base64_calculateSize (ulong_t max);

__LIB_EXPORT ulong_t ecbuf_encode_base64_d (EcBuffer, EcBuffer);

__LIB_EXPORT EcBuffer ecbuf_encode_base64 (EcBuffer);

__LIB_EXPORT EcBuffer ecbuf_decode_base64 (EcBuffer);

__LIB_EXPORT EcBuffer ecbuf_md5 (EcBuffer);

__LIB_EXPORT EcBuffer ecbuf_sha1 (EcBuffer);

__LIB_EXPORT EcBuffer ecbuf_sha_256 (EcBuffer, EcErr);

__LIB_EXPORT EcBuffer ecbuf_xor (EcBuffer, EcBuffer);  // uses always the smallest one

__LIB_EXPORT EcBuffer ecbuf_concat (EcBuffer, EcBuffer);

__LIB_EXPORT EcBuffer ecbuf_hex2bin (EcBuffer);

__LIB_EXPORT EcBuffer ecbuf_bin2hex (EcBuffer);

// getters

__LIB_EXPORT const EcString ecbuf_const_str (const EcBuffer);

__LIB_EXPORT EcString ecbuf_str (EcBuffer*);    // convert to string

__LIB_EXPORT void ecbuf_replace (EcString*, EcBuffer*);

// iterators ----------------------------------------------------------------------------

#pragma pack(push, 16)
typedef struct 
{
  
  EcString line;
  
  char b1;
  
  char b2;
  
  EcBuffer buf;
  
  const unsigned char* pos;
  
} EcBufferIterator;
#pragma pack(pop)

__LIB_EXPORT void ecbuf_iterator (EcBuffer, EcBufferIterator*);

__LIB_EXPORT int ecbufit_readln (EcBufferIterator*);

__LIB_EXPORT void ecbufit_reset (EcBufferIterator*);

__CPP_EXTERN______________________________________________________________________________END

#endif
