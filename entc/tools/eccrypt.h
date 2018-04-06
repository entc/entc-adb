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

#define ENTC_AES_TYPE_CBC                0x01
#define ENTC_AES_TYPE_CFB                0x02

#define ENTC_AES_TYPE_CFB_1              0xF2
#define ENTC_AES_TYPE_CFB_8              0xF3
#define ENTC_AES_TYPE_CFB_128            0xF4

#define ENTC_KEY_PADDING_SHA256          0x00     // default
#define ENTC_KEY_PADDING_ANSI_X923       0x01
#define ENTC_KEY_PADDING_ZEROS           0x02
#define ENTC_KEY_PADDING_PKCS7           0x03
#define ENTC_KEY_PASSPHRASE_DATA         0x04

//=============================================================================

struct EcEncryptAES_s; typedef struct EcEncryptAES_s* EcEncryptAES;

//-----------------------------------------------------------------------------

__LIBEX EcEncryptAES ecencrypt_aes_create (const EcString secret, uint_t cypher_type, uint_t key_type);

__LIBEX void ecencrypt_aes_destroy (EcEncryptAES* pself);

__LIBEX EcBuffer ecencrypt_aes_update (EcEncryptAES, EcBuffer, EcErr);

__LIBEX EcBuffer ecencrypt_aes_finalize (EcEncryptAES, EcErr);

//=============================================================================

struct EcDecryptAES_s; typedef struct EcDecryptAES_s* EcDecryptAES;

//-----------------------------------------------------------------------------

__LIBEX EcDecryptAES ecdecrypt_aes_create (const EcString secret, uint_t cypher_type, uint_t key_type);

__LIBEX void ecdecrypt_aes_destroy (EcDecryptAES* pself);

__LIBEX EcBuffer ecdecrypt_aes_update (EcDecryptAES, EcBuffer, EcErr);

__LIBEX EcBuffer ecdecrypt_aes_finalize (EcDecryptAES, EcErr);

//=============================================================================

__LIBEX int ecencrypt_file (const EcString source, const EcString dest, const EcString secret, unsigned int type, EcErr);

__LIBEX int ecdecrypt_file (const EcString source, const EcString dest, const EcString secret, unsigned int type, EcErr);

#endif
