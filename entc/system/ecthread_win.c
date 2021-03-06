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

#if defined _WIN64 || defined _WIN32

#include "ecthread.h"
#include "ecmutex.h"

#include <windows.h>

//-----------------------------------------------------------------------------------

void ecthread_schedule(ecthread_main_fct main, int argc, char* argv[])
{
  if (main) { main(argc, argv); }
}

//-----------------------------------------------------------------------------------

struct EcThread_s {
  
  HANDLE th;
  
  ecthread_callback_fct fct;
  
  void* ptr;
  
};

//-----------------------------------------------------------------------------------

DWORD WINAPI ecthread_run(LPVOID ptr)
{
  EcThread self = ptr;
  
  if (self->fct)
  {    
    // do the user defined loop
    while (self->fct(self->ptr));
  }  
  return 0;
}

//-----------------------------------------------------------------------------------

EcThread ecthread_new()
{
  EcThread self = ENTC_NEW(struct EcThread_s);
  
  self->th = NULL;

  return self;
}

//-----------------------------------------------------------------------------------

void ecthread_delete(EcThread* pself)
{
  EcThread self = *pself;
  
  ecthread_join(self);
  
  ENTC_DEL(pself, struct EcThread_s);
}

//-----------------------------------------------------------------------------------

void ecthread_start(EcThread self, ecthread_callback_fct fct, void* ptr)
{
  if (self->th == NULL)
  {
    self->fct = fct;
    self->ptr = ptr;
    self->th = CreateThread (NULL, 0, (LPTHREAD_START_ROUTINE)ecthread_run, (LPVOID)self, 0, NULL);
  }
}

//-----------------------------------------------------------------------------------

void ecthread_join(EcThread self)
{  
  if (self->th != NULL) {
    // wait until the thread terminates
    WaitForSingleObject (self->th, INFINITE);
    // release resources
    CloseHandle(self->th);
    self->th = NULL;
  }
}

//-----------------------------------------------------------------------------------

#endif
