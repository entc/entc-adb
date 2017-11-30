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

#include "../system/macros.h"
#include "../system/types.h"

typedef void (_STDCALL *ecthread_main_fct)(int argc, char *argv[]);

__CPP_EXTERN______________________________________________________________________________START

__LIB_EXPORT void ecthread_schedule(ecthread_main_fct, int argc, char* argv[]);

__CPP_EXTERN______________________________________________________________________________END

typedef int (_STDCALL *ecthread_callback_fct)(void* ptr);

struct EcThread_s; typedef struct EcThread_s* EcThread;

__CPP_EXTERN______________________________________________________________________________START

__LIB_EXPORT EcThread ecthread_new(void);

__LIB_EXPORT void ecthread_delete(EcThread*);

__LIB_EXPORT void ecthread_start(EcThread, ecthread_callback_fct, void* ptr);

__LIB_EXPORT void ecthread_join(EcThread);

__LIB_EXPORT void ecthread_cancel(EcThread);

__CPP_EXTERN______________________________________________________________________________END

struct EcTimedThread_s; typedef struct EcTimedThread_s* EcTimedThread;

__CPP_EXTERN______________________________________________________________________________START

__LIB_EXPORT EcTimedThread ectimedthread_new(void);

__LIB_EXPORT void ectimedthread_delete(EcTimedThread*);

__LIB_EXPORT void ectimedthread_start(EcTimedThread, ecthread_callback_fct, void* ptr, ulong_t timeout);

__LIB_EXPORT void ectimedthread_stop(EcTimedThread);

__CPP_EXTERN______________________________________________________________________________END

#endif
