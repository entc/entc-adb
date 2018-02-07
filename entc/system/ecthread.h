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

#ifndef ENTC_SYSTEM_THREAD_H
#define ENTC_SYSTEM_THREAD_H 1

//-----------------------------------------------------------------------------

#include "system/ecdefs.h"

//-----------------------------------------------------------------------------

typedef int (__STDCALL *ecthread_callback_fct)(void* ptr);
typedef void (__STDCALL *ecthread_fct_onDestroy)(void* ptr);

struct EcThread_s; typedef struct EcThread_s* EcThread;

//-----------------------------------------------------------------------------

__LIBEX EcThread ecthread_new (ecthread_fct_onDestroy onDestroy);

__LIBEX void ecthread_delete (EcThread*);

__LIBEX void ecthread_start (EcThread, ecthread_callback_fct, void* ptr);

__LIBEX void ecthread_join (EcThread);

__LIBEX void ecthread_cancel (EcThread);

//-----------------------------------------------------------------------------

__LIBEX void ecthread_sleep (unsigned long milliseconds);

#endif
