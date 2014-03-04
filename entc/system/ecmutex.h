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

#ifndef ENTC_SYSTEM_MUTEX_H
#define ENTC_SYSTEM_MUTEX_H 1

#include "../system/macros.h"

struct EcMutex_s; typedef struct EcMutex_s* EcMutex;
struct EcReadWriteLock_s; typedef struct EcReadWriteLock_s* EcReadWriteLock;

__CPP_EXTERN______________________________________________________________________________START

__LIB_EXPORT EcMutex ecmutex_new();

__LIB_EXPORT void ecmutex_delete(EcMutex*);

__LIB_EXPORT void ecmutex_lock(EcMutex);

__LIB_EXPORT void ecmutex_unlock(EcMutex);

__LIB_EXPORT EcReadWriteLock ecreadwritelock_new();

__LIB_EXPORT void ecreadwritelock_delete(EcReadWriteLock*);

__LIB_EXPORT void ecreadwritelock_lockRead(EcReadWriteLock);

__LIB_EXPORT void ecreadwritelock_unlockRead(EcReadWriteLock);

__LIB_EXPORT void ecreadwritelock_lockWrite(EcReadWriteLock);

__LIB_EXPORT void ecreadwritelock_unlockWrite(EcReadWriteLock);

__CPP_EXTERN______________________________________________________________________________END


#endif
