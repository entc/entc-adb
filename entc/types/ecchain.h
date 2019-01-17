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

#ifndef ENTC_TYPES_CHAIN_H
#define ENTC_TYPES_CHAIN_H 1

#include "types/ecalloc.h"

struct EcChain_s; typedef struct EcChain_s* EcChain;

__ENTC_LIBEX

// constructor / destructor

__ENTC_LIBEX EcChain ecchain_create (EcAlloc);

__ENTC_LIBEX void ecchain_destroy (EcAlloc, EcChain*);

__ENTC_LIBEX void ecchain_clear(EcChain);

// in out

__ENTC_LIBEX uint_t ecchain_add(EcChain, void*);

__ENTC_LIBEX void ecchain_del(EcChain, uint_t);

__ENTC_LIBEX void* ecchain_get(EcChain, uint_t);

// iterator

__ENTC_LIBEX uint_t ecchain_begin(const EcChain);

__ENTC_LIBEX uint_t ecchain_end(const EcChain);

__ENTC_LIBEX uint_t ecchain_next(const EcChain, uint_t);

// misc

__ENTC_LIBEX void printInfo(const EcChain);

__ENTC_LIBEX void ecchain_dumpArray(const EcChain);

__ENTC_LIBEX void ecchain_dumpStack(const EcChain);

__ENTC_LIBEX

#endif
