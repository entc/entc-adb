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

#ifdef __GNUC__

#include <pthread.h>
#include <sys/wait.h>
#include <unistd.h>

//-----------------------------------------------------------------------------------

struct EcThread_s {
  
  ecthread_callback_fct fct;
  
  ecthread_fct_onDestroy onDestroy;
  
  void* ptr;
  
  pthread_t tid;
  
  int status;
    
};

//-----------------------------------------------------------------------------------

static void* ecthread_run (void* params)
{
  EcThread self = params;
  
  if (self->fct)
  {
    while (self->fct (self->ptr))
    {
      pthread_testcancel();
      
      wait(0);
    }
  }
  
  if (self->onDestroy)
  {
    self->onDestroy (self->ptr);
  }
  
  return NULL;
}

//-----------------------------------------------------------------------------------

EcThread ecthread_new (ecthread_fct_onDestroy onDestroy)
{
  EcThread self = ENTC_NEW(struct EcThread_s);
  
  self->fct = NULL;
  self->ptr = NULL;
  //memset(self->tid, 0x00, sizeof(pthread_t));
  
  self->onDestroy = onDestroy;
  
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

void ecthread_cancel (EcThread self)
{
  pthread_cancel (self->tid);
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

//-----------------------------------------------------------------------------------

void ecthread_sleep (unsigned long milliseconds)
{
  usleep (milliseconds * 1000);
}

#endif

//-----------------------------------------------------------------------------------


