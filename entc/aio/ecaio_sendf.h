/*
 * Copyright (c) 2010-2017 "Alexander Kalkhof" [email:entc@kalkhof.org]
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

#ifndef ENTC_SYSTEM_AIO_SENDFILE_H
#define ENTC_SYSTEM_AIO_SENDFILE_H 1

//-----------------------------------------------------------------------------

#include "types/ecerr.h"
#include "types/ecbuffer.h"
#include "aio/ecaio_socket.h"

//=============================================================================

struct EcAioSendFile_s; typedef struct EcAioSendFile_s* EcAioSendFile;

typedef int   (__STDCALL *fct_ecaio_sfile_onInit) (void* ptr, EcRefCountedSocket, uint64_t fileSize, const EcString file, const EcString name, EcErr);
typedef void  (__STDCALL *fct_ecaio_sfile_onDone) (void* ptr, const EcString file, const EcString name);

//-----------------------------------------------------------------------------

__ENTC_LIBEX EcAioSendFile ecaio_sendfile_create (const EcString file, const EcString name, EcRefCountedSocket, void*, fct_ecaio_sfile_onInit, fct_ecaio_sfile_onDone);

__ENTC_LIBEX void ecaio_sendfile_destroy (EcAioSendFile*);

__ENTC_LIBEX void ecaio_sendfile_setBase64 (EcAioSendFile, int base64);

__ENTC_LIBEX int ecaio_sendfile_setSecret (EcAioSendFile, const EcString, unsigned int sectype, EcErr err);

__ENTC_LIBEX int ecaio_sendfile_assign (EcAioSendFile*, EcAio aio, EcErr err);

//=============================================================================

#endif
