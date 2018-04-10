/*
 * Copyright (c) 2010-2018 "Alexander Kalkhof" [email:entc@kalkhof.org]
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

#ifndef ENTC_TOOLS_CODE_H
#define ENTC_TOOLS_CODE_H 1

//=============================================================================

#include "system/ecdefs.h"
#include "types/ecbuffer.h"
#include "types/ecerr.h"

//-----------------------------------------------------------------------------
// hash codes

__LIBEX EcBuffer echash_sha_1 (EcBuffer, EcErr);

__LIBEX EcBuffer echash_sha256 (EcBuffer, EcErr);

__LIBEX EcBuffer echash_md5 (EcBuffer, EcErr);

//-----------------------------------------------------------------------------
// transform codes

__LIBEX EcBuffer eccode_base64_encode (EcBuffer);

__LIBEX EcBuffer eccode_base64_decode (EcBuffer);

//=============================================================================

struct EcBase64Encode_s; typedef struct EcBase64Encode_s* EcBase64Encode;

//-----------------------------------------------------------------------------

__LIBEX EcBase64Encode eccode_base64_encode_create (void);

__LIBEX void eccode_base64_encode_destroy (EcBase64Encode* pself);

__LIBEX uint_t eccode_base64_encode_update (EcBase64Encode, EcBuffer dest, EcBuffer source, EcErr);

__LIBEX uint_t eccode_base64_encode_finalize (EcBase64Encode, EcBuffer dest, EcErr);

__LIBEX uint_t eccode_base64_encode_sourceSize (uint_t size);

//=============================================================================

#endif
