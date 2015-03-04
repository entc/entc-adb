/*
 * Copyright (c) 2010-2015 "Alexander Kalkhof" [email:entc@kalkhof.org]
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

#ifndef ENTC_TOOLS_ASYNCSV_H
#define ENTC_TOOLS_ASYNCSV_H 1

#include "../system/macros.h"
#include "../system/types.h"
#include "../utils/eclogger.h"

#include "system/ecsocket.h"
#include "system/ecevents.h"

struct EcAsyncSvc_s; typedef struct EcAsyncSvc_s* EcAsyncSvc;

struct EcAsyncContext_s; typedef struct EcAsyncContext_s* EcAsyncContext;

typedef int (*async_context_run_fct)(EcAsyncContext, EcAsyncSvc);

typedef void (*async_context_destroy_fct) (void**);

struct EcAsyncContext_s
{
    
  EcHandle handle;

  async_context_run_fct run;
  
  async_context_destroy_fct del;
  
  void* ptr;
  
};

__CPP_EXTERN______________________________________________________________________________START

__LIB_EXPORT EcAsyncSvc ecasyncsvc_create (EcEventContext ec);

__LIB_EXPORT void ecasyncsvc_destroy (EcAsyncSvc*);

__LIB_EXPORT void ecasyncsvc_start (EcAsyncSvc);

__LIB_EXPORT void ecasyncsvc_stop (EcAsyncSvc);

__LIB_EXPORT int ecasyncsvc_add (EcAsyncSvc, EcAsyncContext);

__CPP_EXTERN______________________________________________________________________________END

struct EcAsyncServ_s; typedef struct EcAsyncServ_s* EcAsyncServ;

typedef void* (*serv_context_onCreate_fct) (EcSocket, void*);

typedef void (*serv_context_onDestroy_fct) (void**);

typedef int (*serv_context_onRecv_fct) (void*, char* buffer, int len);

typedef int (*serv_context_onIdle_fct) (void*);

typedef struct 
{
  
  serv_context_onCreate_fct onCreate;
  
  serv_context_onDestroy_fct onDestroy;
  
  serv_context_onRecv_fct onRecvAll;
  
  serv_context_onRecv_fct onRecvPart;

  serv_context_onIdle_fct onIdle;
  
  void* ptr;
  
} EcAsyncServCallbacks;

__CPP_EXTERN______________________________________________________________________________START

__LIB_EXPORT EcAsyncServ ecaserv_create (EcAsyncServCallbacks*);

__LIB_EXPORT void ecaserv_destroy (EcAsyncServ*);

__LIB_EXPORT int ecaserv_start (EcAsyncServ, const EcString host, ulong_t port);

__LIB_EXPORT void ecaserv_stop (EcAsyncServ);

// misc / debug

__LIB_EXPORT void ecaserv_run (EcAsyncServ);

__LIB_EXPORT EcEventContext ecaserv_getEventContext (EcAsyncServ);

__CPP_EXTERN______________________________________________________________________________END

struct EcAsyncServContext_s; typedef struct EcAsyncServContext_s* EcAsyncServContext;

__CPP_EXTERN______________________________________________________________________________START

__LIB_EXPORT void ecaserv_context_recv (EcAsyncServContext, ulong_t len);

__CPP_EXTERN______________________________________________________________________________END

#endif