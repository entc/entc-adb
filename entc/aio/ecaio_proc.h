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

#ifndef ENTC_SYSTEM_AIO_PROC_H
#define ENTC_SYSTEM_AIO_PROC_H 1

//-----------------------------------------------------------------------------

#include "types/ecerr.h"
#include "types/ecbuffer.h"
#include "aio/ecaio.h"

//=============================================================================

struct EcAioProc_s; typedef struct EcAioProc_s* EcAioProc;

//-----------------------------------------------------------------------------

__ENTC_LIBEX EcAioProc ecaio_proc_create (void* handle);

__ENTC_LIBEX int ecaio_proc_assign (EcAioProc*, EcAio aio, EcErr err);

__ENTC_LIBEX void ecaio_proc_setCallback (EcAioProc, void*, fct_ecaio_context_onNotify, fct_ecaio_context_destroy);

//=============================================================================

#endif
