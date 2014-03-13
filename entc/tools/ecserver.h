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

#ifndef ENTC_TOOLS_SERVER_H
#define ENTC_TOOLS_SERVER_H 1

#include "../system/macros.h"

#include "../utils/eclogger.h"
#include "../system/ecsocket.h"
#include "../system/ecevents.h"

typedef int  (_STDCALL *ecserver_callback_fct)(void* ptr, void** object, EcLogger logger);

struct EcServer_s; typedef struct EcServer_s* EcServer;

typedef struct {

  // definition of the accept thread callback, returns an object
  ecserver_callback_fct accept_thread;

  void* accept_ptr;

  // defition of the worker thread, which takes an object an destroys it
  ecserver_callback_fct worker_thread;

  void* worker_ptr;

  // defition of the worker thread, which takes an object an destroys it
  ecserver_callback_fct clear_fct;
  
  void* clear_ptr;

} EcServerCallbacks;

__CPP_EXTERN______________________________________________________________________________START  
  
__LIB_EXPORT EcServer ecserver_new(EcLogger logger, uint_t poolSize, EcServerCallbacks*, EcEventContext);
  
__LIB_EXPORT void ecserver_delete(EcServer*);
  
__LIB_EXPORT int ecserver_start(EcServer);
  
__CPP_EXTERN______________________________________________________________________________END

#endif
