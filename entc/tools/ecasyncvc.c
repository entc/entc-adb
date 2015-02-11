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

void ecasyncsvc_context_destroy (void** pptr)
{
  EcAsyncContext self = *pptr;
  
  if (isAssigned (self->del))
  {
    self->del (&(self->ptr));
  }
  
  ENTC_DEL (pptr, struct EcAsyncContext_s); 
}

//-----------------------------------------------------------------------------------------------------------

EcAsyncSvc ecasyncsvc_create (EcEventContext ec, EcLogger logger)
{
  EcAsyncSvc self = ENTC_NEW (struct EcAsyncSvc_s);
  
  self->ec = ec;
  self->logger = logger;
  
  self->thread = ecthread_new ();
  
  self->queue = ece_list_create (self->ec, ecasyncsvc_context_destroy);
  
  return self;
}

//-----------------------------------------------------------------------------------------------------------

void ecasyncsvc_destroy (EcAsyncSvc* pself)
{
  EcAsyncSvc self = *pself;
  
  ecasyncsvc_stop (self);
  
  ecthread_delete (&(self->thread));
  
  ece_list_destroy (&(self->queue));
  
  ENTC_DEL (pself, struct EcAsyncSvc_s);
}

//-----------------------------------------------------------------------------------------------------------

int ecasyncsvc_add (EcAsyncSvc self, EcAsyncContext context)
{
  return ece_list_add (self->queue, context->handle, ENTC_EVENTTYPE_READ, context);  
}

//-----------------------------------------------------------------------------------------------------------

int _STDCALL ecasyncsvc_run (void* params)
{
  int ret = TRUE;
  
  EcAsyncSvc self = params;
  
  EcAsyncContext context;
    
  int res = ece_list_wait (self->queue, ENTC_INFINTE, (void**)&context, self->logger);  
  if ((res == ENTC_EVENT_TIMEOUT)) // timeout or interupt
  {
    return TRUE;
  }

  if (res < 0) // termination of the process
  {
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
  
  printf("del\n");
  
  ece_list_del (self->queue, context->handle);
      
  return ret;
}

//-----------------------------------------------------------------------------------------------------------

void ecasyncsvc_start (EcAsyncSvc self)
{
  ecthread_start (self->thread, ecasyncsvc_run, (void*)self);
}

//-----------------------------------------------------------------------------------------------------------

void ecasyncsvc_stop (EcAsyncSvc self)
{
  ecthread_join (self->thread); 
}

//-----------------------------------------------------------------------------------------------------------

struct EcAsyncServ_s
{
  
  EcLogger logger;
  
  EcEventContext ec;
  
  EcAsyncSvc svc;
  
  EcSocket socket;
  
  EcAsyncServCallbacks callbacks;
  
  void* ptr;  
  
};

//-----------------------------------------------------------------------------------------------------------

struct EcAsyncServContext_s
{

  EcSocket socket;
  
  char* buffer;
  
  uint_t pos;
  
  uint_t bpos;
  
  uint_t len;
  
  EcAsyncServCallbacks* callbacks;
  
  void* ptr;
  
};

//-----------------------------------------------------------------------------------------------------------

int ecaworker_onIdle (EcAsyncServContext self)
{
  int bytesToRecv = 0;
  
  if (isAssigned (self->callbacks->onIdle))
  {
    bytesToRecv = self->callbacks->onIdle (self->ptr);
  }
  
  if (bytesToRecv > 0)
  {
    ecaserv_context_recv (self, bytesToRecv);
    return TRUE;
  }
  else
  {
    return FALSE;
  }  
}

//-----------------------------------------------------------------------------------------------------------

int ecaworker_onRecvAll (EcAsyncServContext self)
{
  int ret = FALSE;
  
  if (isAssigned (self->callbacks->onRecvAll))
  {
    ret = self->callbacks->onRecvAll (self->ptr, self->buffer, self->len);
  }
  
  return ret;
}

//-----------------------------------------------------------------------------------------------------------

int ecaworker_onRecvPart (EcAsyncServContext self)
{
  // this means continue on collecting data
  int ret = TRUE;
  
  if (isAssigned (self->callbacks->onRecvPart))
  {
    ret = self->callbacks->onRecvPart (self->ptr, self->buffer, self->pos);
    
    // if continue, assume that all data was read out of the buffer
    // set the buffer to start pos
    if (ret)
    {
      self->pos = 0;
    }
  }
  
  return ret;
}

//-----------------------------------------------------------------------------------------------------------

EcAsyncServContext ecaworker_create (EcSocket socket, EcAsyncServCallbacks* callbacks, void* ptr)
{
  EcAsyncServContext self = ENTC_NEW (struct EcAsyncServContext_s);
  
  self->socket = socket;
  self->pos = 0;
  self->buffer = NULL;

  self->callbacks = callbacks;
  self->ptr = ptr;
  
  ecaworker_onIdle (self);
  
  return self;
}

//-----------------------------------------------------------------------------------------------------------

void ecaworker_destroy (EcAsyncServContext* pself)
{
  EcAsyncServContext self = *pself;

  if (isAssigned (self->callbacks->onDestroy))
  {
    self->callbacks->onDestroy (&(self->ptr));
  }  
  
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
    ulong_t res;
    
    ecsocket_resetHandle (ctx->handle);

    res = ecsocket_readBunch (self->socket, self->buffer + self->pos, self->len - self->pos);
    if (res > 0)
    {
      self->pos += res;     
      
      // true: continue with data collecting, false: abort
      if (ecaworker_onRecvPart (self))
      {
        if (self->pos == self->len)
        {
          return ecaworker_onRecvAll (self) && ecaworker_onIdle (self);
        }        
      }
      else 
      {
        return FALSE;
      }
    }
    else
    {
      printf("fuck\n");
      
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
  EcSocket clientSocket;

  EcAsyncServ self = ctx->ptr;
  
  ecsocket_resetHandle (ctx->handle);
  
  clientSocket = ecsocket_accept (self->socket);
  if (isAssigned (clientSocket))
  {  
    EcAsyncContext context = ENTC_NEW (struct EcAsyncContext_s);
    
    context->handle = ecsocket_getReadHandle (clientSocket);

    if (ecasyncsvc_add (self->svc, context))
    {
      void* ptr = NULL;
      
      if ( isAssigned (self->callbacks.onCreate))
      {
        ptr = self->callbacks.onCreate (clientSocket, self->callbacks.ptr);
      }
      
      if (isAssigned (ptr))
      {
        EcAsyncServContext sc = ecaworker_create (clientSocket, &(self->callbacks), ptr);
        
        context->ptr = sc;
        context->run = ecaworker_run;
        context->del = ecaworker_onDel;
        
        eclogger_log(self->logger, LL_TRACE, "ASYN", "client connected" );        
      }
      else
      {
        ecsocket_delete (&clientSocket);        
      }
    }
    else
    {
      ecsocket_delete (&clientSocket);
      //eclogger_log(self->logger, LL_TRACE, "ASYN", "connection refused, maximum reached" );
    }
  }
  return TRUE;
}

//-----------------------------------------------------------------------------------------------------------

EcAsyncServ ecaserv_create (EcLogger logger, EcAsyncServCallbacks* callbacks)
{
  EcAsyncServ self = ENTC_NEW (struct EcAsyncServ_s);

  self->logger = logger;
  
  self->ec = ece_context_new ();
  self->svc = ecasyncsvc_create (self->ec, logger);
  
  // callbacks
  memcpy(&(self->callbacks), callbacks, sizeof(EcAsyncServCallbacks));
    
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
  
  ecasyncsvc_stop (self->svc);
  // close listen socket
  ecsocket_delete(&(self->socket));
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
