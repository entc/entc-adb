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

#ifndef ENTC_SYSTEM_EVENTS_H
#define ENTC_SYSTEM_EVENTS_H 1

#include "../system/macros.h"
#include "../types/ecstring.h"
#include "../utils/eclogger.h"

#include "ecmutex.h"

#define ENTC_INFINTE           1000000
#define ENTC_EVENTTYPE_READ    0
#define ENTC_EVENTTYPE_WRITE   1
#define ENTC_EVENTTYPE_USER    2

#define ENTC_EVENT_TIMEOUT     0
#define ENTC_EVENT_ABORT       1

#define ENTC_EVENT_UNKNOWN    -1
#define ENTC_EVENT_ERROR      -2

#if defined _WIN64 || defined _WIN32

// windows version of HANDLE
typedef void* EcHandle;

#else

// unix version
typedef int EcHandle;

#endif

struct EcEventContext_s; typedef struct EcEventContext_s* EcEventContext;

__CPP_EXTERN______________________________________________________________________________START

__LIB_EXPORT EcEventContext ece_context_new (void);

__LIB_EXPORT void ece_context_delete (EcEventContext*);

__LIB_EXPORT int ece_context_waitforTermination (EcEventContext, uint_t timeout);

__LIB_EXPORT void ece_context_triggerTermination (EcEventContext);

__CPP_EXTERN______________________________________________________________________________END
 
struct EcEventQueue_s; typedef struct EcEventQueue_s* EcEventQueue;

__CPP_EXTERN______________________________________________________________________________START

__LIB_EXPORT EcEventQueue ece_queue_new (EcEventContext);
  
__LIB_EXPORT void ece_queue_delete (EcEventQueue*);
  
__LIB_EXPORT void ece_queue_add (EcEventQueue, EcHandle, int type);

__LIB_EXPORT int ece_queue_wait(EcEventQueue, uint_t timeout, EcLogger logger);

// handle methods

__LIB_EXPORT EcHandle ece_queue_gen (EcEventQueue);

__LIB_EXPORT void ece_queue_set (EcEventQueue, EcHandle);

__LIB_EXPORT void ece_queue_del (EcEventQueue, EcHandle*);

__CPP_EXTERN______________________________________________________________________________END

typedef void (*events_callback_fct)(void* ptr);

struct EcEventFiles_s; typedef struct EcEventFiles_s* EcEventFiles;

__CPP_EXTERN______________________________________________________________________________START
  
__LIB_EXPORT EcEventFiles ece_files_new(EcLogger logger);
  
__LIB_EXPORT void ece_files_delete(EcEventFiles*);
  
  
#ifdef __APPLE_CC__
__LIB_EXPORT void ece_files_register(EcEventFiles, int fd, int filter, int fflags, events_callback_fct onChange, void* onChangePtr, events_callback_fct onDelete, void* onDeletePtr);
#else
__LIB_EXPORT void ece_files_register(EcEventFiles, const EcString, events_callback_fct onChange, void* onChangePtr, events_callback_fct onDelete, void* onDeletePtr);  
#endif
  
__CPP_EXTERN______________________________________________________________________________END


#endif
