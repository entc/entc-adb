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

#include "ecasyncvc.h"

#include "system/ecmutex.h"
#include "system/ecthread.h"

//-----------------------------------------------------------------------------------------------------------

typedef struct {
  
  EcSocket socket;
  
  EcHandle handle;
  
  ecasync_worker_rawdata_cb callback;
  
  void* ptr;
  
  EcBuffer buffer;
  
} EcAsyncWorkerContext;

//-----------------------------------------------------------------------------------------------------------

static void _STDCALL ecasync_worker_destroy (void** ptr)
{
  EcAsyncWorkerContext* self = *ptr;
  
  ecsocket_delete (&(self->socket));
  
  ecbuf_destroy (&(self->buffer));
  
  ENTC_DEL (ptr, EcAsyncWorkerContext);
}

//-----------------------------------------------------------------------------------------------------------

static EcHandle _STDCALL ecasync_worker_handle (void* ptr)
{
  EcAsyncWorkerContext* self = ptr;
  
  return self->handle;
}

//-----------------------------------------------------------------------------------------------------------

static int _STDCALL ecasync_worker_run (void* ptr)
{
  EcAsyncWorkerContext* self = ptr;

  ulong_t res = ecsocket_readBunch (self->socket, self->buffer->buffer, self->buffer->size);
  if (res > 0)
  {
    if (isAssigned (self->callback))
    {
      self->callback (self->ptr, self->buffer->buffer, res);
    }
    
    return TRUE;
  }

  // eclogger_msg (LL_WARN, "ENTC", "async", "connection reset by peer");
  return FALSE;
}

//-----------------------------------------------------------------------------------------------------------

EcAsyncContext ecasync_worker_create (EcSocket sock, ulong_t timeout, ecasync_worker_rawdata_cb callback, void* ptr)
{
  static const EcAsyncContextCallbacks callbacks = {ecasync_worker_destroy, ecasync_worker_handle, ecasync_worker_run};
  
  EcAsyncWorkerContext* self = ENTC_NEW (EcAsyncWorkerContext);
  
  self->socket = sock;
  self->handle = ecsocket_getReadHandle (sock);
  
  self->callback = callback;
  self->ptr = ptr;
  
  self->buffer = ecbuf_create (1024);  // good
  
  return ecasync_context_create (timeout, &callbacks, self);  
}

//-----------------------------------------------------------------------------------------------------------

typedef struct {
  
  EcSocket socket;
  
  EcHandle handle;
  
  ecasync_worker_idle_cb idle;
  
  ecasync_worker_recv_cb recv;
  
  void* ptr;
  
  unsigned char* buffer;
  
  ulong_t pos;
  
  ulong_t len;
  
} EcAsyncStrictWorkerContext;

//-----------------------------------------------------------------------------------------------------------

static void _STDCALL ecasync_strict_worker_destroy (void** ptr)
{
  EcAsyncStrictWorkerContext* self = *ptr;
  
  ecsocket_delete (&(self->socket));
  
  
  ENTC_DEL (ptr, EcAsyncStrictWorkerContext);
}

//-----------------------------------------------------------------------------------------------------------

static EcHandle _STDCALL ecasync_strict_worker_handle (void* ptr)
{
  EcAsyncStrictWorkerContext* self = ptr;
  
  return self->handle;
}

//-----------------------------------------------------------------------------------------------------------

int ecasync_strict_worker_onIdle (EcAsyncStrictWorkerContext* self)
{
  int bytesToRecv = 0;
  
  if (isAssigned (self->idle))
  {
    bytesToRecv = self->idle (self->ptr);
  }
  
  if (bytesToRecv > 0)
  {
    self->pos = 0;
    self->len = bytesToRecv;
    
    if (isAssigned (self->buffer))
    {
      self->buffer = realloc (self->buffer, bytesToRecv + 1);
    }
    else
    {
      self->buffer = ENTC_MALLOC (bytesToRecv + 1);
    }
    return TRUE;
  }
  else
  {
    return FALSE;
  }    
}

//-----------------------------------------------------------------------------------------------------------

int ecasync_strict_worker_onRecv (EcAsyncStrictWorkerContext* self)
{
  if (isAssigned (self->recv))
  {
    return self->recv (self->ptr, self->buffer, self->len);
  }  
  
  return FALSE;
}

//-----------------------------------------------------------------------------------------------------------

static int _STDCALL ecasync_strict_worker_run (void* ptr)
{
  EcAsyncStrictWorkerContext* self = ptr;
  
  if (self->pos < self->len)
  {
    ulong_t res;
    
    ecsocket_resetHandle (self->handle);
    
    res = ecsocket_readBunch (self->socket, self->buffer + self->pos, self->len - self->pos);
    if (res > 0)
    {
      self->pos += res;     
      
      if (self->pos == self->len)
      {
        return ecasync_strict_worker_onRecv (self) && ecasync_strict_worker_onIdle (self);
      }        
    }
    else
    {
      // eclogger_msg (LL_WARN, "ENTC", "async", "connection reset by peer");
      return FALSE;
    }    
  }
  else
  {
    return ecasync_strict_worker_onIdle (self) && ecasync_strict_worker_run (self);
  }
  
  return TRUE;  
}

//-----------------------------------------------------------------------------------------------------------

EcAsyncContext _STDCALL ecasync_strict_worker_create (EcSocket sock, ulong_t timeout, ecasync_worker_idle_cb idlecb, ecasync_worker_recv_cb recvcb, void* ptr)
{
  static const EcAsyncContextCallbacks callbacks = {ecasync_strict_worker_destroy, ecasync_strict_worker_handle, ecasync_strict_worker_run};
  
  EcAsyncStrictWorkerContext* self = ENTC_NEW (EcAsyncStrictWorkerContext);
  
  self->socket = sock;
  self->handle = ecsocket_getReadHandle (sock);
  
  self->idle = idlecb;
  self->recv = recvcb;
  self->ptr = ptr;
  
  self->buffer = NULL;  // dynamic allocation
  self->pos = 0;
  self->len = 0;
  
  ecasync_strict_worker_onIdle (self);
  
  return ecasync_context_create (timeout, &callbacks, self);  
}

//-----------------------------------------------------------------------------------------------------------

typedef struct {
  
  EcSocket socket;
  
  EcHandle handle;
  
  EcAsync async;
  
  ecasync_accept_worker_cb cb;
  
  void* ptr;
  
} EcAsyncAcceptContext;

//-----------------------------------------------------------------------------------------------------------

static void _STDCALL ecasync_accept_destroy (void** ptr)
{
  EcAsyncAcceptContext* self = *ptr;
  
  ecsocket_delete (&(self->socket));
  
  ENTC_DEL (ptr, EcAsyncAcceptContext);
}

//-----------------------------------------------------------------------------------------------------------

static EcHandle _STDCALL ecasync_accept_handle (void* ptr)
{
  EcAsyncAcceptContext* self = ptr;

  return self->handle;
}

//-----------------------------------------------------------------------------------------------------------

static int _STDCALL ecasync_accept_run (void* ptr)
{
  EcAsyncAcceptContext* self = ptr;
  
  EcSocket clientSocket;
  
  ecsocket_resetHandle (self->handle);
  
  clientSocket = ecsocket_accept (self->socket);
  if (isAssigned (clientSocket))
  {  
    if (isAssigned (self->cb))
    {
      EcAsyncContext context = self->cb (self->ptr, clientSocket);
      ecasync_addSingle (self->async, &context);
    }
    else
    {
      ecsocket_delete (&clientSocket);
    }
  }
  
  return TRUE;
}

//-----------------------------------------------------------------------------------------------------------

EcAsyncContext ecasync_accept_create (const EcString host, ulong_t port, EcEventContext ec, EcAsync async, ecasync_accept_worker_cb cb, void* ptr)
{
  static const EcAsyncContextCallbacks callbacks = {ecasync_accept_destroy, ecasync_accept_handle, ecasync_accept_run};
  
  // try to bind socket
  EcSocket sock = ecsocket_new (ec);

  if( !ecsocket_listen (sock, host, port) )
  {
    ecsocket_delete (&sock);
    
    return NULL;
  }
  
  EcAsyncAcceptContext* self = ENTC_NEW (EcAsyncAcceptContext);
    
  self->socket = sock;
  self->handle = ecsocket_getAcceptHandle (sock);
  
  self->async = async;
  self->cb = cb;
  self->ptr = ptr;
  
  return ecasync_context_create (ENTC_INFINITE, &callbacks, self);  
}

//-----------------------------------------------------------------------------------------------------------
