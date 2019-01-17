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

#ifndef ENTC_SYSTEM_SHM_H
#define ENTC_SYSTEM_SHM_H 1

#include "sys/entc_export.h"
#include "sys/entc_types.h"
#include "types/ecalloc.h"

struct EcShm_s; typedef struct EcShm_s* EcShm;

__ENTC_LIBEX

__ENTC_LIBEX EcShm ecshm_create (EcAlloc alloc, uint32_t key, uint32_t size);

__ENTC_LIBEX void ecshm_destroy (EcShm*);

__ENTC_LIBEX void* ecshm_get (EcShm);

__ENTC_LIBEX int ecshm_wasCreated (EcShm);

__ENTC_LIBEX

struct EcSem_s; typedef struct EcSem_s* EcSem;

__ENTC_LIBEX

__ENTC_LIBEX EcSem ecsem_create (EcAlloc alloc, uint32_t key);

__ENTC_LIBEX void ecsem_destroy (EcSem*);

__ENTC_LIBEX void ecsem_clear (EcSem);

__ENTC_LIBEX void ecsem_waitAndSet (EcSem, int waitFor, int setVal);

__ENTC_LIBEX void ecsem_wait (EcSem, int waitFor);

__ENTC_LIBEX void ecsem_set (EcSem, int setVal);

__ENTC_LIBEX int ecsem_tryDec (EcSem);

__ENTC_LIBEX void ecsem_queue_wait (EcSem);

__ENTC_LIBEX void ecsem_queue_send (EcSem);

__ENTC_LIBEX

#endif
