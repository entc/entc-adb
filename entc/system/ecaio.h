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

#ifndef ENTC_SYSTEM_AIO_H
#define ENTC_SYSTEM_AIO_H 1

//-----------------------------------------------------------------------------

#include "types/ecerr.h"
#include "types/ecbuffer.h"
#include "system/ecaio_ctx.h"

//=============================================================================

struct EcAio_s; typedef struct EcAio_s* EcAio;

//-----------------------------------------------------------------------------

__LIBEX EcAio ecaio_create (void);

__LIBEX void ecaio_destroy (EcAio*);

__LIBEX int ecaio_init (EcAio, EcErr);

__LIBEX int ecaio_append (EcAio, void* handle, EcAioContext ctx, EcErr);

__LIBEX int ecaio_abort (EcAio, EcErr);

__LIBEX int ecaio_addQueueEvent (EcAio, void* ptr, fct_ecaio_context_process, fct_ecaio_context_destroy, EcErr);

__LIBEX int ecaio_wait (EcAio, unsigned long timeout, EcErr);

__LIBEX int ecaio_wait_abortOnSignal (EcAio, int onlyTerm, EcErr);

__LIBEX int ecaio_addContextToEvent (EcAio, EcAioContext ctx, EcErr err);

//-----------------------------------------------------------------------------
// special events

__LIBEX int ecaio_appendVNode (EcAio, int fd, void* data, EcErr err);

__LIBEX int ecaio_appendPNode (EcAio, int pid, void* data, EcErr err);

__LIBEX int ecaio_appendENode (EcAio, EcAioContext ctx, void** eh, EcErr err);

__LIBEX int ecaio_triggerENode (EcAio, void* eh, EcErr err);

//-----------------------------------------------------------------------------

#endif
