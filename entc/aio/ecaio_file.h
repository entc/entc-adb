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

#ifndef ENTC_SYSTEM_AIO_FILE_H
#define ENTC_SYSTEM_AIO_FILE_H 1

//-----------------------------------------------------------------------------

#include "types/ecerr.h"
#include "aio/ecaio.h"

//=============================================================================

struct EcAioFileReader_s; typedef struct EcAioFileReader_s* EcAioFileReader;

//-----------------------------------------------------------------------------

__ENTC_LIBEX EcAioFileReader ecaio_filereader_create (void* handle);

__ENTC_LIBEX int ecaio_filereader_assign (EcAioFileReader*, EcAio aio, EcErr err);

__ENTC_LIBEX void ecaio_filereader_setCallback (EcAioFileReader, void*, fct_ecaio_context_onInit, fct_ecaio_context_onRead, fct_ecaio_context_destroy);

//=============================================================================

struct EcAioFileWriter_s; typedef struct EcAioFileWriter_s* EcAioFileWriter;

//-----------------------------------------------------------------------------

__ENTC_LIBEX EcAioFileWriter ecaio_filewriter_create (void* handle);

__ENTC_LIBEX int ecaio_filewriter_assign (EcAioFileWriter*);

__ENTC_LIBEX void ecaio_filewriter_setBufferCP (EcAioFileWriter, const char* buffer, unsigned long size);

__ENTC_LIBEX void ecaio_filewriter_setBufferBT (EcAioFileWriter, EcBuffer*);

//=============================================================================

#endif
