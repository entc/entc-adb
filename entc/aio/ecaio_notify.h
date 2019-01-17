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

#ifndef ENTC_SYSTEM_AIO_NOTIFY_H
#define ENTC_SYSTEM_AIO_NOTIFY_H 1

//-----------------------------------------------------------------------------

#include "types/ecerr.h"
#include "types/ecbuffer.h"
#include "aio/ecaio.h"

//-----------------------------------------------------------------------------

#define ENTC_AIO_NOTIFY_NONE        0x0000
#define ENTC_AIO_NOTIFY_WRITE       0x0001
#define ENTC_AIO_NOTIFY_SIZE        0x0002
#define ENTC_AIO_NOTIFY_ATTR        0x0004
#define ENTC_AIO_NOTIFY_DIR         0x0008
#define ENTC_AIO_NOTIFY_FILE        0x0010
#define ENTC_AIO_NOTIFY_DELETE      0x0020

//=============================================================================

struct EcAioNotify_s; typedef struct EcAioNotify_s* EcAioNotify;

//-----------------------------------------------------------------------------

__ENTC_LIBEX EcAioNotify ecaio_notify_create ();

__ENTC_LIBEX void ecaio_notify_destroy (EcAioNotify*);

__ENTC_LIBEX int ecaio_notify_init (EcAioNotify, const char* path, EcErr);

__ENTC_LIBEX int ecaio_notify_assign (EcAioNotify*, EcAio aio, EcErr err);

__ENTC_LIBEX void ecaio_notify_setCallback (EcAioNotify, void*, fct_ecaio_context_onNotify, fct_ecaio_context_destroy);

//=============================================================================

#endif
