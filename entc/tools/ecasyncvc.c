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

#include "ecasyncvc.h"

#include "system/ecmutex.h"
#include "system/ecthread.h"

//-----------------------------------------------------------------------------------------------------------

struct EcAsyncSvc_s
{
  
  // references
  
  EcEventContext ec;

  EcLogger logger;

  // owned
  
  EcThread thread;
  
  EcEventQueue queue;
  
  EcHandle interupt;
  
};

//-----------------------------------------------------------------------------------------------------------

EcAsyncSvc ecasyncsvc_create (EcEventContext ec, EcLogger logger)
{
  EcAsyncSvc self = ENTC_NEW (struct EcAsyncSvc_s);
  
  self->ec = ec;
  self->logger = logger;
  
  self->thread = ecthread_new ();
  
  self->queue = ece_list_create (self->ec);
  
  return self;
}

//-----------------------------------------------------------------------------------------------------------

void ecasyncsvc_destroy (EcAsyncSvc* pself)
{
  EcAsyncSvc self = *pself;
  
  ecthread_join (self->thread);
  
  ecthread_delete (&(self->thread));
  
  ENTC_DEL (pself, struct EcAsyncSvc_s);
}

//-----------------------------------------------------------------------------------------------------------

void ecasyncsvc_add (EcAsyncSvc self, EcAsyncContext context)
{
  ece_list_add (self->queue, context->handle, ENTC_EVENTTYPE_READ, context);  
}

//-----------------------------------------------------------------------------------------------------------

int ecasyncsvc_run (void* params)
{
  int ret = TRUE;
  
  EcAsyncSvc self = params;
  
  EcAsyncContext context;
    
  int res = ece_list_wait (self->queue, ENTC_INFINTE, (void**)&context, self->logger);
  
  printf("res:%i\n", res);
  
  if ((res == ENTC_EVENT_TIMEOUT)) // timeout or interupt
  {
    printf("timeout\n");
    return TRUE;
  }

  if (res < 0) // termination of the process
  {
    printf("abort\n");
    return FALSE;  // tell the thread to terminate
  }
  
  if (isNotAssigned (context))
  {
    return FALSE;
  }
  
  if (context->run (context, self))
  {
    return TRUE;
  }
  
  ece_list_del (self->queue, context->handle);
  
  ecsocket_closeHandle (context->handle);
  
  if (isAssigned (context->del))
  {
    context->del (&(context->ptr));
  }
  
  ENTC_DEL (&context, struct EcAsyncContext_s);
  
  return ret;
}

//-----------------------------------------------------------------------------------------------------------

void ecasyncsvc_start (EcAsyncSvc self)
{
  ecthread_start(self->thread, ecasyncsvc_run, (void*)self);
}

//-----------------------------------------------------------------------------------------------------------

struct EcAsyncServ_s
{
  
  EcLogger logger;
  
  EcEventContext ec;
  
  EcAsyncSvc svc;
  
  EcSocket socket;
  
};

//-----------------------------------------------------------------------------------------------------------

struct EcAsyncServContext_s
{

  EcSocket socket;
  
  char* buffer;
  
  uint_t pos;
  
  uint_t len;
  
};

//-----------------------------------------------------------------------------------------------------------

int ecaworker_onIdle (EcAsyncServContext self)
{
  ecaserv_context_recv (self, 20);
  
  return TRUE;
}

//-----------------------------------------------------------------------------------------------------------

int ecaworker_onRecv (EcAsyncServContext self)
{
  
  return FALSE;
}

//-----------------------------------------------------------------------------------------------------------

EcAsyncServContext ecaworker_create (EcSocket socket)
{
  EcAsyncServContext self = ENTC_NEW (struct EcAsyncServContext_s);
  
  self->socket = socket;
  self->pos = 0;
  self->buffer = NULL;

  ecaworker_onIdle (self);
  
  return self;
}

//-----------------------------------------------------------------------------------------------------------

void ecaworker_destroy (EcAsyncServContext* pself)
{
  EcAsyncServContext self = *pself;

  printf("delete context\n");
  
  ecsocket_delete (&(self->socket));

  if (isAssigned (self->buffer))
  {
    ENTC_FREE (self->buffer);
  }
  
  ENTC_DEL (pself, struct EcAsyncServContext_s);
}

//-----------------------------------------------------------------------------------------------------------

void ecaserv_context_recv (EcAsyncServContext self, ulong_t len)
{
  self->pos = 0;
  self->len = len;
  
  if (isAssigned (self->buffer))
  {
    self->buffer = realloc (self->buffer, len + 1);
  }
  else
  {
    self->buffer = ENTC_MALLOC (len + 1);
  }
}

//-----------------------------------------------------------------------------------------------------------

int ecaworker_run (EcAsyncContext ctx, EcAsyncSvc svc)
{
  EcAsyncServContext self = ctx->ptr;
  
  if (self->pos < self->len)
  {
    ecsocket_resetHandle (ctx->handle);

    ulong_t res = ecsocket_read (self->socket, self->buffer + self->pos, self->len - self->pos);
    if (res > 0)
    {
      self->pos += res;
      
      printf("read %lu bytes to total %lu\n", res, self->pos);
      
      if (self->pos == self->len)
      {
        return ecaworker_onRecv (self) && ecaworker_onIdle (self);
      }
    }
    else
    {
      printf("read error %lu\n", res);

      return FALSE;
    }    
  }
  else
  {
    return ecaworker_onIdle (self) && ecaworker_run (ctx, svc);
  }

  return TRUE;
}

//-----------------------------------------------------------------------------------------------------------

void ecaworker_onDel (void** pptr)
{
  ecaworker_destroy ((EcAsyncServContext*)pptr);
}

//-----------------------------------------------------------------------------------------------------------

int ecaserv_accept (EcAsyncContext ctx, EcAsyncSvc svc)
{
  EcAsyncServ self = ctx->ptr;
  
  ecsocket_resetHandle (ctx->handle);
  
  EcSocket clientSocket = ecsocket_accept (self->socket);
  if (isAssigned (clientSocket))
  {
    EcAsyncServContext sc = ecaworker_create (clientSocket);
    
    EcAsyncContext context = ENTC_NEW (struct EcAsyncContext_s);
    
    context->handle = ecsocket_getReadHandle (clientSocket);

    context->ptr = sc;
    context->run = ecaworker_run;
    context->del = ecaworker_onDel;
    
    ecasyncsvc_add (self->svc, context);
    
    eclogger_log(self->logger, LL_TRACE, "ASYN", "client connected" );
  }
  
  return TRUE;
}

//-----------------------------------------------------------------------------------------------------------

EcAsyncServ ecaserv_create (EcLogger logger)
{
  EcAsyncServ self = ENTC_NEW (struct EcAsyncServ_s);

  self->logger = logger;
  
  self->ec = ece_context_new ();
  self->svc = ecasyncsvc_create (self->ec, logger);
    
  return self;
}

//-----------------------------------------------------------------------------------------------------------

void ecaserv_destroy (EcAsyncServ* pself)
{
  EcAsyncServ self = *pself;
  
  ecasyncsvc_destroy(&(self->svc));
  
  ece_context_delete(&(self->ec));

  ENTC_DEL (pself, struct EcAsyncServ_s);
}

//-----------------------------------------------------------------------------------------------------------

int ecaserv_start (EcAsyncServ self, const EcString host, ulong_t port)
{
  self->socket = ecsocket_new (self->ec, self->logger);
  if( !ecsocket_listen (self->socket, host, port) )
  {
    ecsocket_delete(&(self->socket));
    return FALSE;
  }
  
  {
    EcAsyncContext context = ENTC_NEW (struct EcAsyncContext_s);
    
    context->handle = ecsocket_getAcceptHandle (self->socket);

    context->ptr = self;
    context->run = ecaserv_accept;
    context->del = NULL;
    
    ecasyncsvc_add (self->svc, context);
  }
  
  ecasyncsvc_start (self->svc);
  
  return TRUE;
}

//-----------------------------------------------------------------------------------------------------------

void ecaserv_stop (EcAsyncServ self)
{
  ece_context_triggerTermination (self->ec);
}

//-----------------------------------------------------------------------------------------------------------

void ecaserv_run (EcAsyncServ self)
{
  ece_context_waitforTermination (self->ec, ENTC_INFINTE);
}

//-----------------------------------------------------------------------------------------------------------

EcEventContext ecaserv_getEventContext (EcAsyncServ self)
{
  return self->ec;
}

//-----------------------------------------------------------------------------------------------------------
