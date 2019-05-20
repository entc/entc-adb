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

#include "entc_thread.h"

#ifdef __GNUC__

#include <pthread.h>
#include <sys/wait.h>
#include <unistd.h>

#include "sys/entc_types.h"

//-----------------------------------------------------------------------------------

struct EntcThread_s
{  
  entc_thread_worker_fct fct;
  
  void* ptr;
  
  pthread_t tid;
  
  int status;
};

//-----------------------------------------------------------------------------------

static void* entc_thread_run (void* params)
{
  EntcThread self = params;
  
  if (self->fct)
  {
    while (self->fct (self->ptr))
    {
      pthread_testcancel();
      
      wait(0);
    }
  }
  
  return NULL;
}

//-----------------------------------------------------------------------------------

EntcThread entc_thread_new (void)
{
  EntcThread self = ENTC_NEW(struct EntcThread_s);
  
  self->fct = NULL;
  self->ptr = NULL;
  //memset(self->tid, 0x00, sizeof(pthread_t));
  
  self->status = FALSE;
  
  return self;
}

//-----------------------------------------------------------------------------------

void entc_thread_del (EntcThread* pself)
{
  EntcThread self = *pself;
  
  if (self->status)
  {
    // WARNING
    entc_thread_join (self);
  }
  
  ENTC_DEL(pself, struct EntcThread_s);
}

//-----------------------------------------------------------------------------------

void entc_thread_cancel (EntcThread self)
{
  pthread_cancel (self->tid);
}

//-----------------------------------------------------------------------------------

void entc_thread_start (EntcThread self, entc_thread_worker_fct fct, void* ptr)
{
  // define some special attributes
  pthread_attr_t attr;
  pthread_attr_init(&attr);
  pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
  // assign the callback parameters  
  self->fct = fct;
  self->ptr = ptr;
  // finally create the thread
  self->status = (pthread_create(&(self->tid), &attr, entc_thread_run, self) == 0);
}

//-----------------------------------------------------------------------------------

void entc_thread_join (EntcThread self)
{
  if (self->status) {
    void* status;
    pthread_join(self->tid, &status);
    
    self->status = FALSE;
  }
}

//-----------------------------------------------------------------------------

void entc_thread_sleep (unsigned long milliseconds)
{
  usleep (milliseconds * 1000);
}

#elif defined _WIN64 || defined _WIN32

#include <windows.h>

//-----------------------------------------------------------------------------

struct EntcThread_s
{  
  HANDLE th;
  
  entc_thread_worker_fct fct;
  
  void* ptr;
};

//-----------------------------------------------------------------------------------

DWORD WINAPI entc_thread_run (LPVOID ptr)
{
  EntcThread self = ptr;
  
  if (self->fct)
  {    
    // do the user defined loop
    while (self->fct(self->ptr));
  }  
  return 0;
}

//-----------------------------------------------------------------------------------

EntcThread entc_thread_new (void)
{
  EntcThread self = ENTC_NEW(struct EntcThread_s);
  
  self->th = NULL;
  
  return self;
}

//-----------------------------------------------------------------------------------

void entc_thread_del (EntcThread* pself)
{
  EntcThread self = *pself;
  
  entc_thread_join(self);
  
  ENTC_DEL(pself, struct EntcThread_s);
}

//-----------------------------------------------------------------------------------

void entc_thread_start (EntcThread self, entc_thread_callback_fct fct, void* ptr)
{
  if (self->th == NULL)
  {
    self->fct = fct;
    self->ptr = ptr;
    self->th = CreateThread (NULL, 0, (LPTHREAD_START_ROUTINE)entc_thread_run, (LPVOID)self, 0, NULL);
  }
}

//-----------------------------------------------------------------------------------

void entc_thread_join (EntcThread self)
{  
  if (self->th != NULL)
  {
    // wait until the thread terminates
    WaitForSingleObject (self->th, INFINITE);
    // release resources
    CloseHandle(self->th);
    self->th = NULL;
  }
}

//-----------------------------------------------------------------------------------

void entc_thread_cancel (EntcThread self)
{
  if (self->th != NULL)
  {
    if (TerminateThread(self->th, 0) == 0)
    {
      EcErr err = ecerr_create ();
      
      ecerr_lastErrorOS (err, ENTC_LVL_ERROR);
      
      eclog_fmt (LL_ERROR, "ENTC", "thread", "can't cancel thread: %s", err->text);
      
      ecerr_destroy (&err);
    }
  }
}

//-----------------------------------------------------------------------------------

void entc_thread_sleep (unsigned long milliseconds)
{
  Sleep (milliseconds);
}

#endif

//-----------------------------------------------------------------------------


