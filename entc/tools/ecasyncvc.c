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
  
  EcMutex mutex;
  
  EcThread thread;
  
  EcList ctxlist;
  
  EcEventContext ec;
  
  EcEventQueue queue;
  
  EcHandle interupt;
  
  EcLogger logger;
  
};

//-----------------------------------------------------------------------------------------------------------

EcAsyncSvc ecasyncsvc_create (void)
{
  EcAsyncSvc self = ENTC_NEW (struct EcAsyncSvc_s);
  
  self->mutex = ecmutex_new ();
  self->ctxlist = eclist_new ();
  self->thread = ecthread_new ();
  
  self->queue = NULL;
  self->interupt = 0;
  
  return self;
}

//-----------------------------------------------------------------------------------------------------------

void ecasyncsvc_destroy (EcAsyncSvc* pself)
{
  EcAsyncSvc self = *pself;
  
  ecthread_join(self->thread);
  
  ecmutex_delete (&(self->mutex));
  ecthread_delete(&(self->thread));
  
  ENTC_DEL (pself, struct EcAsyncSvc_s);
}

//-----------------------------------------------------------------------------------------------------------

void ecasyncsvc_add (EcAsyncSvc self, EcAsyncContext* context)
{
  ecmutex_lock (self->mutex);
  
  eclist_append(self->ctxlist, context);
  
  if (isAssigned (self->queue) && self->interupt)
  {
    ece_queue_set (self->queue, self->interupt);
  }

  ecmutex_unlock (self->mutex);
}

//-----------------------------------------------------------------------------------------------------------

int ecasyncsvc_run (void* params)
{
  int ret = TRUE;
  
  EcListNode* ctxs;
  
  EcAsyncSvc self = params;
  
  ecmutex_lock (self->mutex);

  self->queue = ece_queue_new (self->ec);
  self->interupt = ece_queue_gen (self->queue);
  
  {
    EcListNode node;
    int i = 0;
    
    ctxs = ENTC_MALLOC (eclist_size(self->ctxlist) * sizeof(EcListNode));
    
    for (node = eclist_first(self->ctxlist); node != eclist_end(self->ctxlist); node = eclist_next(node), i++)
    {
      EcAsyncContext* context = eclist_data (node);
      
      ece_queue_add(self->queue, context->handle, ENTC_EVENTTYPE_READ);
      
      ctxs [i] = node;
    }
    
  }
  
  ecmutex_unlock (self->mutex);

  while (TRUE)
  {
    EcListNode node;
    
    int res = ece_queue_wait (self->queue, ENTC_INFINTE, self->logger);
    
    if (res == ENTC_EVENT_ABORT) // termination of the process
    {
      ret = FALSE;  // tell the thread to terminate
      break;
    }
    if ((res == ENTC_EVENT_TIMEOUT) || (res == 2)) // timeout or interupt
    {
      break;
    }
    
    node = ctxs [res - 3];
    {    
      EcAsyncContext* context = eclist_data (node);
    
      if (context->fct (context->ptr, self))
      {
        continue;
      }
    }    
    // delete this context
    ecmutex_lock (self->mutex);

    eclist_erase (node);
    
    ecmutex_unlock (self->mutex);
    
    break;
  }
  
  ecmutex_lock (self->mutex);

  ece_queue_delete (&(self->queue));
  self->interupt = 0;
  
  ecmutex_unlock (self->mutex);

  ENTC_FREE (ctxs);
  
  return ret;
}

//-----------------------------------------------------------------------------------------------------------

void ecasyncsvc_start (EcAsyncSvc self)
{
  ecthread_start(self->thread, ecasyncsvc_run, (void*)self);
}

//-----------------------------------------------------------------------------------------------------------
