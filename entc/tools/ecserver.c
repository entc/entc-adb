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

#include "ecserver.h"

#include "../system/ecthread.h"

typedef struct {
  
  // Owned
  
  EcLogger logger;
  
  EcThread thread;
  
  // Reference
  
  EcServer server;
  
  
} EcServerThread;

struct EcServer_s
{
  // reference
  EcLogger logger;
  
  EcEventContext mainabort;
  
  void* eventlist;
  
  uint_t poolSize;
  
  EcServerThread* threads;
  
  EcServerCallbacks callbacks;

  // queuing

  EcEventQueue equeue;

  EcList queue;

  EcMutex mutex;

  EcHandle worker_lock;

};

/*------------------------------------------------------------------------*/

int ecserver_accept_run (void* params)
{
  void* object = NULL;
  EcServerThread* self = params;
  // check if we have a callback method
  if (isNotAssigned (self->server->callbacks.accept_thread))
  {
    eclogger_log (self->logger, LL_ERROR, "QSRV", "{accept} no accept callback is set -> thread terminates");
    return FALSE;
  }
  // accept thread callback
  if( !self->server->callbacks.accept_thread(self->server->callbacks.accept_ptr, &object, self->logger) )
  {  
    // signaled to stop the thread
    eclogger_log(self->logger, LL_DEBUG, "QSRV", "{accept} aborted");
    return FALSE;     
  }

  {
    uint_t pending = 0;
    
    ecmutex_lock(self->server->mutex);
  
    // needs to add object to the queue
    eclist_append (self->server->queue, object);
    pending = eclist_size (self->server->queue);
  
    ecmutex_unlock(self->server->mutex);

    eclogger_logformat (self->logger, LL_TRACE, "QSRV", "{accept} Received object '%p' -> added to queue (pending: %u)", object, pending);

    ece_queue_set (self->server->equeue, self->server->worker_lock);
  }
  return TRUE;
}

/*------------------------------------------------------------------------*/
                            
int ecserver_worker_run (void* params)
{
  EcServerThread* self = params;
  // check if we have a callback method
  if(self->server->callbacks.worker_thread)
  {
    void* object;
    EcListNode node;
    int res;

    eclogger_log(self->logger, LL_TRACE, "QSRV", "{worker} wait on queue");

    res = ece_queue_wait (self->server->equeue, ENTC_INFINTE, self->logger);
    // check the return
    if (res == ENTC_EVENT_ABORT)
    {
      // termination of the process
      eclogger_log(self->logger, LL_TRACE, "QSRV", "{worker} aborted");
      return FALSE;
    }
    // wait until we got something in the queue to do
    ecmutex_lock(self->server->mutex);

    node = eclist_first(self->server->queue);

    if (node == eclist_end(self->server->queue))
    {
      // no items/objects available
      ecmutex_unlock(self->server->mutex);
      return TRUE;
    }

    eclogger_log(self->logger, LL_TRACE, "QSRV", "{worker} Found object in queue");

    object = eclist_data(node);

    eclist_erase(node);

    ecmutex_unlock(self->server->mutex);

    // trigger other threads to continue
    ece_queue_set (self->server->equeue, self->server->worker_lock);

    if( !self->server->callbacks.worker_thread (self->server->callbacks.worker_ptr, &object, self->logger) )
    {
      eclogger_log(self->logger, LL_TRACE, "QSRV", "{worker} aborted");
      // signaled to stop the thread
      return FALSE;
    }    
  }
  return TRUE;
}

/*------------------------------------------------------------------------*/

EcServer ecserver_new(EcLogger logger, uint_t poolSize, EcServerCallbacks* callbacks, EcEventContext ec)
{
  EcServer self = ENTC_NEW(struct EcServer_s);
  // init
  self->logger = logger;
  self->mainabort = ec;
  self->poolSize = poolSize;
  self->equeue = ece_queue_new (ec);
  self->queue = eclist_new();
  self->mutex = ecmutex_new();
  self->worker_lock = ece_queue_gen (self->equeue);

  memcpy(&(self->callbacks), callbacks, sizeof(EcServerCallbacks));

  self->threads = NULL;
  
  return self;
}

/*------------------------------------------------------------------------*/

void ecserver_delete(EcServer* ptr)
{
  EcServer self = *ptr;
  uint_t i;
  EcListNode node;
  
  if( self->threads != NULL )
  {
    for (i = 0; i < (self->poolSize + 1); i++)
    {
      EcServerThread* st = &(self->threads[i]);
      
      eclogger_log(self->logger, LL_TRACE, "QSRV", "{server} wait for thread to terminate");
      
      ecthread_join(st->thread);
      
      ecthread_delete(&(st->thread));
            
      eclogger_log(self->logger, LL_TRACE, "QSRV", "{server} thread removed");

      eclogger_del (&(st->logger));
    }
    free(self->threads);
    self->threads = NULL;    
  }
  
  for (node = eclist_first(self->queue); node != eclist_end(self->queue); node = eclist_next(node))
  {
    void* object = eclist_data(node);

    if (isAssigned (self->callbacks.clear_fct))
    {
      self->callbacks.clear_fct (NULL, &object, self->logger);
    }
  }
    
  eclist_delete(&(self->queue));
  ecmutex_delete(&(self->mutex));
  
  ece_queue_delete (&(self->equeue));
      
  ENTC_DEL(ptr, struct EcServer_s);
}

/*------------------------------------------------------------------------*/

int ecserver_start(EcServer self)
{
  uint_t i;
  
  self->threads = malloc(sizeof(EcServerThread) * (self->poolSize + 1));
  memset(self->threads, 0x0, sizeof(EcServerThread) * (self->poolSize + 1));
  
  // add event
 // eceventqueue_add(self->equeue, self->ehandle, Ec_EVENTTYPE_READ);
  // create the accept thread
  {
    EcServerThread* st = &(self->threads[0]);

    st->thread = ecthread_new();  
    st->logger = eclogger_new(1);
    st->server = self;
    
    eclogger_sync(st->logger, self->logger);
    
    ecthread_start(st->thread, ecserver_accept_run, (void*)st);
  }
  // start all threads
  for(i = 1; i < (self->poolSize + 1); i++)
  {
    EcServerThread* st = &(self->threads[i]);
    
    st->thread = ecthread_new();  
    st->logger = eclogger_new(i + 1);
    st->server = self;
    
    eclogger_sync(st->logger, self->logger);

    ecthread_start(st->thread, ecserver_worker_run, (void*)st);
  }  
  
  return TRUE;
}

/*------------------------------------------------------------------------*/
