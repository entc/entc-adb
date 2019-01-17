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

#ifndef ENTC_SYSTEM_AIO_MSG_H
#define ENTC_SYSTEM_AIO_MSG_H 1

//-----------------------------------------------------------------------------

#include "types/ecerr.h"
#include "types/ecbuffer.h"
#include "aio/ecaio.h"

//=============================================================================

struct EcMsgChannel_s; typedef struct EcMsgChannel_s* EcMsgChannel;

//-----------------------------------------------------------------------------

__ENTC_LIBEX EcMsgChannel ecmsg_channel_create ();

__ENTC_LIBEX void ecmsg_channel_destroy (EcMsgChannel*);

__ENTC_LIBEX int ecmsg_channel_init (EcMsgChannel, const EcString name, EcErr err);

__ENTC_LIBEX void* ecmsg_channel_handle (EcMsgChannel);

//=============================================================================

struct EcAioMsgReader_s; typedef struct EcAioMsgReader_s* EcAioMsgReader;

//-----------------------------------------------------------------------------

__ENTC_LIBEX EcAioMsgReader ecaio_msgreader_create (void* channelHandle);

__ENTC_LIBEX int ecaio_msgreader_assign (EcAioMsgReader*, EcAio aio, EcErr err);

__ENTC_LIBEX void ecaio_msgreader_setCallback (EcAioMsgReader, void*, fct_ecaio_context_onRead, fct_ecaio_context_destroy);

//=============================================================================

#endif
