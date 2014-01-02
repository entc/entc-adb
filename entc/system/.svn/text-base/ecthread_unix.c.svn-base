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

#include "ecthread.h"
#include "ecmutex.h"
#include "ecevents.h"

#ifdef __GNUC__

#include <pthread.h>

//-----------------------------------------------------------------------------------

void ecthread_schedule(ecthread_main_fct main, int argc, char* argv[])
{
  if (main) { main(argc, argv); }
}

//-----------------------------------------------------------------------------------

struct EcThread_s {
  
  ecthread_callback_fct fct;
  
  void* ptr;
  
  pthread_t tid;
  
  int status;
    
};

//-----------------------------------------------------------------------------------

void* ecthread_run(void* params)
{
  EcThread self = params;
  
  if (self->fct)
  {
    while (self->fct(self->ptr))
    {
      wait(0);
    }
  }  
  pthread_exit(0);
}

//-----------------------------------------------------------------------------------

EcThread ecthread_new()
{
  EcThread self = ENTC_NEW(struct EcThread_s);
  
  self->fct = NULL;
  self->ptr = NULL;
  //memset(self->tid, 0x00, sizeof(pthread_t));
  
  self->status = FALSE;
  
  return self;
}

//-----------------------------------------------------------------------------------

void ecthread_delete(EcThread* pself)
{
  EcThread self = *pself;
  
  if (self->status) {
    // WARNING
    ecthread_join (self);
  }
  
  ENTC_DEL(pself, struct EcThread_s);
}

//-----------------------------------------------------------------------------------

void ecthread_start(EcThread self, ecthread_callback_fct fct, void* ptr)
{
  // define some special attributes
  pthread_attr_t attr;
  pthread_attr_init(&attr);
  pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
  // assign the callback parameters  
  self->fct = fct;
  self->ptr = ptr;
  // finally create the thread
  self->status = (pthread_create(&(self->tid), &attr, ecthread_run, self) == 0);
}

//-----------------------------------------------------------------------------------

void ecthread_join (EcThread self)
{
  if (self->status) {
    void* status;
    pthread_join(self->tid, &status);
    
    self->status = FALSE;
  }
}

#endif

//-----------------------------------------------------------------------------------

struct EcTimedThread_s
{
  
  EcThread thread;
  
  EcEventContext ec;
  
  ulong_t timeout;
  
  ecthread_callback_fct fct;
  
  void* ptr;
  
};

//-----------------------------------------------------------------------------------

EcTimedThread ectimedthread_new()
{
  EcTimedThread self = ENTC_NEW(struct EcTimedThread_s);
  
  self->thread = ecthread_new();
  self->ec = ece_context_new();
  
  return self;
}

//-----------------------------------------------------------------------------------

void ectimedthread_delete(EcTimedThread* pself)
{
  EcTimedThread self = *pself;
  
  ecthread_delete(&(self->thread));
  ece_context_delete(&(self->ec));
  
  ENTC_DEL(pself, struct EcTimedThread_s);
}

//-----------------------------------------------------------------------------------

int ectimedthread_run(void* params)
{
  EcTimedThread self = params;

  if (!self->fct)
  {
    return FALSE;
  }
    
  if (ece_context_waitforTermination (self->ec, self->timeout) == ENTC_EVENT_TIMEOUT)
  {
    self->fct(self->ptr);
    return TRUE;
  }
  else
  {    
    return FALSE;
  }
}

//-----------------------------------------------------------------------------------

void ectimedthread_start(EcTimedThread self, ecthread_callback_fct fct, void* ptr, ulong_t timeout)
{
  self->timeout = timeout;
  self->fct = fct;
  self->ptr = ptr;
  ecthread_start(self->thread, ectimedthread_run, self);
}

//-----------------------------------------------------------------------------------

void ectimedthread_stop(EcTimedThread self)
{
  // signal to stop thread
  ece_context_triggerTermination (self->ec);
  // wait until thread stopped
  ecthread_join (self->thread);
}

//-----------------------------------------------------------------------------------


