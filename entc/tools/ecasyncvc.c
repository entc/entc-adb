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
#include "system/ectime.h"

#include "types/ecmap.h"

//-----------------------------------------------------------------------------------------------------------

typedef struct {
  
  EcSocket socket;
  
  EcHandle handle;
  
  ecasync_worker_rawdata_cb callback;
  
  void* ptr;
  
  EcBuffer buffer;
  
  EcStopWatch stopwatch;
    
} EcAsyncWorkerContext;

//-----------------------------------------------------------------------------------------------------------

static void _STDCALL ecasync_worker_destroy (void** ptr)
{
  EcAsyncWorkerContext* self = *ptr;
  
  ecsocket_delete (&(self->socket));
  
  ecbuf_destroy (&(self->buffer));
  
  ecstopwatch_destroy (&(self->stopwatch));
    
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
    
    ecstopwatch_start (self->stopwatch);

    return TRUE;
  }

  // eclogger_msg (LL_WARN, "ENTC", "async", "connection reset by peer");
  return FALSE;
}

//-----------------------------------------------------------------------------------------------------------

static int _STDCALL ecasync_worker_hasTimedOut (void* obj, void* ptr)
{
  EcAsyncWorkerContext* self = obj;
  EcStopWatch refWatch = ptr;
  
  return ecstopwatch_timedOutRef(self->stopwatch, refWatch);
}

//-----------------------------------------------------------------------------------------------------------

EcAsyncContext ecasync_worker_create (EcSocket sock, ulong_t timeout, ecasync_worker_rawdata_cb callback, void* ptr)
{
  static const EcAsyncContextCallbacks callbacks = {ecasync_worker_destroy, ecasync_worker_handle, ecasync_worker_run, ecasync_worker_hasTimedOut};
  
  EcAsyncWorkerContext* self = ENTC_NEW (EcAsyncWorkerContext);
  
  self->socket = sock;
  self->handle = ecsocket_getReadHandle (sock);
  
  self->callback = callback;
  self->ptr = ptr;
  
  self->buffer = ecbuf_create (1024);  // good
  
  self->stopwatch = ecstopwatch_create (timeout);  
  ecstopwatch_start (self->stopwatch);
  
  return ecasync_context_create (&callbacks, self);  
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
  
  EcStopWatch stopwatch;
  
} EcAsyncStrictWorkerContext;

//-----------------------------------------------------------------------------------------------------------

static void _STDCALL ecasync_strict_worker_destroy (void** ptr)
{
  EcAsyncStrictWorkerContext* self = *ptr;
  
  ecsocket_delete (&(self->socket));
  
  ecstopwatch_destroy (&(self->stopwatch));
  
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

static int _STDCALL ecasync_strict_worker_hasTimedOut (void* obj, void* ptr)
{
  EcAsyncStrictWorkerContext* self = obj;
  EcStopWatch refWatch = ptr;
  
  return ecstopwatch_timedOutRef(self->stopwatch, refWatch);
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
  
  ecstopwatch_start (self->stopwatch);

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
  static const EcAsyncContextCallbacks callbacks = {ecasync_strict_worker_destroy, ecasync_strict_worker_handle, ecasync_strict_worker_run, ecasync_strict_worker_hasTimedOut};

  EcAsyncStrictWorkerContext* self = ENTC_NEW (EcAsyncStrictWorkerContext);
  
  self->socket = sock;
  self->handle = ecsocket_getReadHandle (sock);
  
  self->idle = idlecb;
  self->recv = recvcb;
  self->ptr = ptr;
  
  self->buffer = NULL;  // dynamic allocation
  self->pos = 0;
  self->len = 0;
  
  self->stopwatch = ecstopwatch_create (timeout);  
  ecstopwatch_start (self->stopwatch);
  
  ecasync_strict_worker_onIdle (self);
  
  return ecasync_context_create (&callbacks, self);  
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
  EcAsyncAcceptContext* self;

  static const EcAsyncContextCallbacks callbacks = {ecasync_accept_destroy, ecasync_accept_handle, ecasync_accept_run, NULL};
  
  // try to bind socket
  EcSocket sock = ecsocket_new (ec, ENTC_SOCKET_PROTOCOL_TCP);

  if( !ecsocket_listen (sock, host, port) )
  {
    ecsocket_delete (&sock);
    
    return NULL;
  }
  
  self = ENTC_NEW (EcAsyncAcceptContext);
    
  self->socket = sock;
  self->handle = ecsocket_getAcceptHandle (sock);
  
  self->async = async;
  self->cb = cb;
  self->ptr = ptr;
  
  return ecasync_context_create (&callbacks, self);  
}

//============================================================================================================

struct EcAsyncUdpContext_s
{
  
  EcDatagram dg;
  
  ecasync_worker_udp_cb recvCb;
  
  ecasync_worker_destroy_cb destCb;
  
  void* ptr;
  
  EcStopWatch stopwatch;
  
  EcMutex mutex;
  
};

//-----------------------------------------------------------------------------------------------------------

EcAsyncUdpContext ecasync_udpcontext_create (ulong_t timeout, ecasync_worker_udp_cb recvcb, ecasync_worker_destroy_cb destroycb, void* ptr)
{
  EcAsyncUdpContext self = ENTC_NEW (struct EcAsyncUdpContext_s);
  
  self->dg = NULL;
  
  self->recvCb = recvcb;
  self->destCb = destroycb;
  self->ptr = ptr;
  
  self->stopwatch = ecstopwatch_create (timeout);  
  ecstopwatch_start (self->stopwatch);
  
  self->mutex = ecmutex_new ();

  return self;
}

//-----------------------------------------------------------------------------------------------------------

void ecasync_udpcontext_destroy (EcAsyncUdpContext* pself)
{
  EcAsyncUdpContext self = *pself;

  if (self->destCb)
  {
    self->destCb (&(self->ptr));
  }
  
  if (self->dg)
  {
    ecdatagram_destroy (&(self->dg));
  }
  
  ecstopwatch_destroy (&(self->stopwatch));
  
  ecmutex_delete(&(self->mutex));
  
  ENTC_DEL (pself, struct EcAsyncUdpContext_s);
}

//-----------------------------------------------------------------------------------------------------------

int ecasync_udpcontext_recv (EcAsyncUdpContext self, EcDatagram* dg, int count)
{
  int res = FALSE;
  
  ecmutex_lock(self->mutex);
  
  if (self->dg)
  {
    ecdatagram_destroy (&(self->dg));
  }
  
  self->dg = *dg;
  *dg = NULL;
  
  ecstopwatch_start (self->stopwatch);
  
  if (isAssigned (self->recvCb))
  {
    res = self->recvCb (self->ptr, self, self->dg, count);
  }
  
  ecmutex_unlock(self->mutex);

  return res;
}

//-----------------------------------------------------------------------------------------------------------

void ecasync_udpcontext_send (EcAsyncUdpContext self, EcBuffer buf, ssize_t len)
{
  ecmutex_lock(self->mutex);

  if (self->dg && buf)
  {
    // copy from one buffer into the next
    ecdatagram_writeBuf (self->dg, buf, len);
  }
  
  ecmutex_unlock(self->mutex);
}

//-----------------------------------------------------------------------------------------------------------

int ecasync_udpcontext_timedOut (EcAsyncUdpContext self, EcStopWatch ref)
{
  return ecstopwatch_timedOutRef(self->stopwatch, ref);
}

//-----------------------------------------------------------------------------------------------------------

struct EcAsynUdpDispatcher_s {
  
  EcSocket socket;
  
  EcHandle handle;
  
  ecasync_dispatcher_cb cb;
  
  void* ptr;
  
  EcMap contexts;
  
  EcMutex mutex;
  
  EcStopWatch stopwatch;
  
};

//-----------------------------------------------------------------------------------------------------------

EcAsynUdpDispatcher ecasync_udpdisp_create (const EcString host, ulong_t port, EcEventContext ec, ecasync_dispatcher_cb cb, void* ptr)
{
  EcAsynUdpDispatcher self = NULL;
  
  // try to bind socket
  EcSocket sock = ecsocket_new (ec, ENTC_SOCKET_PROTOCOL_UDP);
  
  if( !ecsocket_listen (sock, host, port) )
  {
    ecsocket_delete (&sock);
    
    return NULL;
  }
  
  self = ENTC_NEW (struct EcAsynUdpDispatcher_s);

  self->socket = sock;
  self->handle = ecsocket_getAcceptHandle (sock);
  
  self->cb = cb;
  self->ptr = ptr;
  
  self->contexts = ecmap_new ();
  self->mutex = ecmutex_new ();
  
  self->stopwatch = ecstopwatch_create (60000);  // check each 60 seconds
  ecstopwatch_start (self->stopwatch);
    
  return self;
}

//-----------------------------------------------------------------------------------------------------------

static void _STDCALL ecasync_udpdisp_destroy (void** ptr)
{
  EcAsynUdpDispatcher self = *ptr;
  
  ecmutex_delete (&(self->mutex));
  
  ENTC_DEL (ptr, struct EcAsynUdpDispatcher_s);
}

//-----------------------------------------------------------------------------------------------------------

static EcHandle _STDCALL ecasync_udpdisp_handle (void* ptr)
{
  EcAsynUdpDispatcher self = ptr;
  
  return self->handle;  
}

//-----------------------------------------------------------------------------------------------------------

static int _STDCALL ecasync_udpdisp_run (void* ptr)
{
  EcAsynUdpDispatcher self = ptr;
  
  EcDatagram dg = ecdatagram_create (self->socket);
  
  ssize_t count = ecdatagram_read (dg);
  
  if (count == 0)
  {
    ecdatagram_destroy (&dg);
    return TRUE;
  }
  
  // get ident
  const EcString ident = ecdatagram_ident (dg);
  
  ecmutex_lock (self->mutex);  
  
  EcAsyncUdpContext context = NULL;
  
  // check if ident is known
  EcMapNode node = ecmap_find(self->contexts, ident);
  
  if (node == ecmap_end(self->contexts))
  {
    if (isAssigned (self->cb))
    {
      context = self->cb (self->ptr);
      
      node = ecmap_append(self->contexts, ident, context);  
    }
  }
  else
  {
    context = ecmap_data (node);
  }
  
  ecmutex_unlock (self->mutex);
  
  int res = TRUE;
  
  if (context)
  {
    res = ecasync_udpcontext_recv (context, &dg, count);
  }
  else
  {
    ecdatagram_destroy (&dg);    
  }
  
  if (!res)
  {
    ecasync_udpcontext_destroy (&context);

    // we can close this context
    ecmutex_lock (self->mutex);
        
    ecmap_erase(node);
    
    ecmutex_unlock (self->mutex);
  }
  
  return TRUE;  // always true
}

//-----------------------------------------------------------------------------------------------------------

static int _STDCALL ecasync_udpdisp_hasTimedOut (void* obj, void* ptr)
{
  EcAsynUdpDispatcher self = obj;
  EcStopWatch refWatch = ptr;
  
  ecmutex_lock (self->mutex);
  
  if (ecstopwatch_timedOutRef(self->stopwatch, refWatch))
  {
    ecstopwatch_start (self->stopwatch);
    
    EcMapNode node;
    
    for (node = ecmap_first(self->contexts); node != ecmap_end(self->contexts); node = ecmap_next(node))
    {
      EcAsyncUdpContext context = ecmap_data(node);
      
      // bug doesn't work
      /*
      if (ecasync_udpcontext_timedOut (context, refWatch))
      {
        eclogger_msg (LL_WARN, "ENTC", "async", "context timed out");

        ecasync_udpcontext_destroy (&context);
        node = ecmap_erase (node);      
      }
       */
    }
  }
  
  ecmutex_unlock (self->mutex);
  
  return FALSE;  // always false
}

//-----------------------------------------------------------------------------------------------------------

void ecasync_udpdisp_broadcast (EcAsynUdpDispatcher self, EcBuffer buf, ssize_t len, EcAsyncUdpContext ctx)
{
  EcMapNode node;

  ecmutex_lock (self->mutex);

  for (node = ecmap_first(self->contexts); node != ecmap_end(self->contexts); node = ecmap_next(node))
  {
    EcAsyncUdpContext context = ecmap_data(node);

    if (context != ctx)
    {
      ecasync_udpcontext_send (context, buf, len);
    }
  }
  
  ecmutex_unlock (self->mutex);
}

//-----------------------------------------------------------------------------------------------------------

EcAsyncContext ecasync_udpdisp_context (EcAsynUdpDispatcher* self)
{
  static const EcAsyncContextCallbacks callbacks = {ecasync_udpdisp_destroy, ecasync_udpdisp_handle, ecasync_udpdisp_run, ecasync_udpdisp_hasTimedOut};

  EcAsyncContext ret = ecasync_context_create (&callbacks, *self);
  
  *self = NULL;
  
  return ret;
}

//-----------------------------------------------------------------------------------------------------------
