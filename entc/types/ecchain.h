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

#include "../system/macros.h"
#include "../system/types.h"

struct EcChain_s; typedef struct EcChain_s* EcChain;

__CPP_EXTERN______________________________________________________________________________START

// constructor / destructor

__LIB_EXPORT EcChain ecchain_new();

__LIB_EXPORT void ecchain_delete(EcChain*);

__LIB_EXPORT void ecchain_clear(EcChain);

// in out

__LIB_EXPORT uint_t ecchain_add(EcChain, void*);

__LIB_EXPORT void ecchain_del(EcChain, uint_t);

__LIB_EXPORT void* ecchain_get(EcChain, uint_t);

// iterator

__LIB_EXPORT uint_t ecchain_begin(const EcChain);

__LIB_EXPORT uint_t ecchain_end(const EcChain);

__LIB_EXPORT uint_t ecchain_next(const EcChain, uint_t);

// misc

__LIB_EXPORT void printInfo(const EcChain);

__LIB_EXPORT void ecchain_dumpArray(const EcChain);

__LIB_EXPORT void ecchain_dumpStack(const EcChain);

__CPP_EXTERN______________________________________________________________________________END

#endif
