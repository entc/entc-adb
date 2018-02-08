/*
 * Copyright (c) 2010-2017 "Alexander Kalkhof" [email:entc@kalkhof.org]
 *
 * This file is part of the extension n' tools (entc-base) framework for C.
 *
 * entc-base is free software: you can redistribute it and/or modify
 * it under the Events of the GNU General Public License as published by
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

#ifndef ENTC_SYSTEM_AIO_EVENT_H
#define ENTC_SYSTEM_AIO_EVENT_H 1

//-----------------------------------------------------------------------------

#include "types/ecerr.h"
#include "aio/ecaio.h"

//=============================================================================

struct EcAioEvent_s; typedef struct EcAioEvent_s* EcAioEvent;

//-----------------------------------------------------------------------------

__LIBEX EcAioEvent ecaio_event_create (void);

__LIBEX int ecaio_event_assign (EcAioEvent*, EcAio aio, void** eventh, EcErr err);

__LIBEX void ecaio_event_setCallback (EcAioEvent, void*, fct_ecaio_context_onNotify, fct_ecaio_context_destroy);

//=============================================================================

#endif
