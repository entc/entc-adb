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

#ifndef ENTC_TOOLS_CRYPT_H
#define ENTC_TOOLS_CRYPT_H 1

//=============================================================================

#include "system/ecdefs.h"
#include "types/ecudc.h"
#include "types/ecstring.h"

//=============================================================================

struct EcEncryptAES_s; typedef struct EcEncryptAES_s* EcEncryptAES;

//-----------------------------------------------------------------------------

__LIBEX EcEncryptAES ecencrypt_aes_initialize (const EcString secret, EcErr);

__LIBEX EcBuffer ecencrypt_aes_update (EcEncryptAES, EcBuffer, EcErr);

__LIBEX EcBuffer ecencrypt_aes_finalize (EcEncryptAES, EcErr);

__LIBEX void ecencrypt_aes_destroy (EcEncryptAES* pself);

//=============================================================================

struct EcDecryptAES_s; typedef struct EcDecryptAES_s* EcDecryptAES;

//-----------------------------------------------------------------------------

__LIBEX EcDecryptAES ecdecrypt_aes_initialize (const EcString secret, EcErr);

__LIBEX EcBuffer ecdecrypt_aes_update (EcDecryptAES, EcBuffer, EcErr);

__LIBEX EcBuffer ecdecrypt_aes_finalize (EcDecryptAES, EcErr);

__LIBEX void ecdecrypt_aes_destroy (EcDecryptAES* pself);

//=============================================================================

__LIBEX int ecencrypt_file (const EcString source, const EcString dest, const EcString secret, EcErr);

__LIBEX int ecdecrypt_file (const EcString source, const EcString dest, const EcString secret, EcErr);

#endif
