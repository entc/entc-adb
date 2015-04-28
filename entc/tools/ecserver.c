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
  
  EcThread thread;
  
  // Reference
  
  EcServer server;
  
  
} EcServerThread;

struct EcServer_s
{
  
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

int _STDCALL ecserver_accept_run (void* params)
{
  void* object = NULL;
  EcServerThread* self = params;
  // check if we have a callback method
  if (isNotAssigned (self->server->callbacks.accept_thread))
  {
    eclogger_msg (LL_ERROR, "ENTC", "ecserver", "no accept callback is set -> thread terminates");
    return FALSE;
  }
  // accept thread callback
  if (!self->server->callbacks.accept_thread(self->server->callbacks.accept_ptr, &object))
  {  
    // signaled to stop the thread
    eclogger_msg (LL_DEBUG, "ENTC", "ecserver", "accept aborted");
    return FALSE;     
  }

  {
    uint_t pending = 0;
    
    ecmutex_lock(self->server->mutex);
  
    // needs to add object to the queue
    eclist_append (self->server->queue, object);
    pending = eclist_size (self->server->queue);
  
    ecmutex_unlock(self->server->mutex);

    eclogger_fmt (LL_TRACE, "ENTC", "server", "Received object '%p' -> added to queue (pending: %u)", object, pending);

    ece_list_set (self->server->equeue, self->server->worker_lock);
  }
  return TRUE;
}

/*------------------------------------------------------------------------*/
                            
int _STDCALL ecserver_worker_run (void* params)
{
  EcServerThread* self = params;
  // check if we have a callback method
  if (self->server->callbacks.worker_thread)
  {
    void* object = NULL;
    EcListNode node;
    int res;

    eclogger_msg (LL_TRACE, "ENTC", "ecserver", "wait on queue");

    res = ece_list_wait (self->server->equeue, ENTC_INFINTE, NULL);
    // check the return
    if (res == ENTC_EVENT_ABORT)
    {
      // termination of the process
      eclogger_msg (LL_TRACE, "ENTC", "ecserver", "wait aborted");
      return FALSE;
    }
    // wait until we got something in the queue to do
    ecmutex_lock(self->server->mutex);

    node = eclist_first(self->server->queue);

    if (node == eclist_end(self->server->queue))
    {
      eclogger_msg (LL_TRACE, "ENTC", "ecserver", "no items in queue");
      // no items/objects available
      ecmutex_unlock(self->server->mutex);
      return TRUE;
    }

    eclogger_msg (LL_TRACE, "ENTC", "ecserver", "found object in queue");

    object = eclist_data(node);

    eclist_erase(node);

    ecmutex_unlock(self->server->mutex);

    // trigger other threads to continue
    ece_list_set (self->server->equeue, self->server->worker_lock);

    if (!self->server->callbacks.worker_thread (self->server->callbacks.worker_ptr, &object))
    {
      eclogger_msg (LL_TRACE, "ENTC", "ecserver", "process aborted");
      // signaled to stop the thread
      return FALSE;
    }    
  }
  else
  {
    return FALSE;
  }
  return TRUE;
}

/*------------------------------------------------------------------------*/

EcServer ecserver_create (uint_t poolSize, EcServerCallbacks* callbacks, EcEventContext ec)
{
  EcServer self = ENTC_NEW(struct EcServer_s);
  // init
  self->mainabort = ec;
  self->poolSize = poolSize;
  self->equeue = ece_list_create (ec, NULL);
  self->queue = eclist_new();
  self->mutex = ecmutex_new();
  self->worker_lock = ece_list_handle (self->equeue, NULL);

  memcpy(&(self->callbacks), callbacks, sizeof(EcServerCallbacks));

  self->threads = NULL;
  
  return self;
}

/*------------------------------------------------------------------------*/

void ecserver_destroy (EcServer* ptr)
{
  EcServer self = *ptr;
  uint_t i;
  EcListNode node;
  
  if( self->threads != NULL )
  {
    for (i = 0; i < (self->poolSize + 1); i++)
    {
      EcServerThread* st = &(self->threads[i]);
      
      eclogger_msg (LL_TRACE, "ENTC", "ecserver", "wait for thread to terminate");
      
      ecthread_join(st->thread);
      
      ecthread_delete(&(st->thread));
            
      eclogger_msg (LL_TRACE, "ENTC", "ecserver", "thread removed");
    }
    free(self->threads);
    self->threads = NULL;    
  }
  
  for (node = eclist_first(self->queue); node != eclist_end(self->queue); node = eclist_next(node))
  {
    void* object = eclist_data(node);

    if (isAssigned (self->callbacks.clear_fct))
    {
      self->callbacks.clear_fct (NULL, &object);
    }
  }
    
  eclist_delete(&(self->queue));
  ecmutex_delete(&(self->mutex));
  
  ece_list_destroy (&(self->equeue));
      
  ENTC_DEL(ptr, struct EcServer_s);
}

/*------------------------------------------------------------------------*/

int ecserver_start (EcServer self)
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
    st->server = self;
    
    ecthread_start(st->thread, ecserver_accept_run, (void*)st);
  }
  // start all threads
  for(i = 1; i < (self->poolSize + 1); i++)
  {
    EcServerThread* st = &(self->threads[i]);
    
    st->thread = ecthread_new();  
    st->server = self;
    
    ecthread_start(st->thread, ecserver_worker_run, (void*)st);
  }  
  
  return TRUE;
}

/*------------------------------------------------------------------------*/
