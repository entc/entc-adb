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

#include "ecmutex.h"

#include <windows.h>

//-----------------------------------------------------------------------------------

struct EcMutex_s {
  
  CRITICAL_SECTION cs;
  
};

//-----------------------------------------------------------------------------------

EcMutex ecmutex_new()
{
  EcMutex self = ENTC_NEW(struct EcMutex_s);
  
  InitializeCriticalSection(&(self->cs));
  
  return self;
}

//-----------------------------------------------------------------------------------

void ecmutex_delete(EcMutex* pself)
{
  EcMutex self = *pself;
  
  DeleteCriticalSection(&(self->cs));
  
  ENTC_DEL(pself, struct EcMutex_s);
}

//-----------------------------------------------------------------------------------

void ecmutex_lock(EcMutex self)
{
  EnterCriticalSection(&(self->cs));
}

//-----------------------------------------------------------------------------------

void ecmutex_unlock(EcMutex self)
{
  LeaveCriticalSection(&(self->cs));
}

//-----------------------------------------------------------------------------------

struct EcReadWriteLock_s
{
  
  CRITICAL_SECTION cs_r;  
  CRITICAL_SECTION cs_w;
  
  int counter;
  
};

//-----------------------------------------------------------------------------------

EcReadWriteLock ecreadwritelock_new (void)
{
  EcReadWriteLock self = ENTC_NEW(struct EcReadWriteLock_s);
  
  InitializeCriticalSection(&(self->cs_r));
  InitializeCriticalSection(&(self->cs_w));

  self->counter = 0;
  
  return self;  
}

//-----------------------------------------------------------------------------------

void ecreadwritelock_delete (EcReadWriteLock* pself)
{
  EcReadWriteLock self = *pself;
  
  DeleteCriticalSection(&(self->cs_r));
  DeleteCriticalSection(&(self->cs_w));
  
  ENTC_DEL(pself, struct EcReadWriteLock_s);  
}

//-----------------------------------------------------------------------------------

void ecreadwritelock_lockRead (EcReadWriteLock self)
{
  EnterCriticalSection(&(self->cs_r));
  
  if (self->counter == 0)
  {
    EnterCriticalSection(&(self->cs_w));
  }
  
  self->counter++;
  
  LeaveCriticalSection(&(self->cs_r));
}

//-----------------------------------------------------------------------------------

void ecreadwritelock_unlockRead (EcReadWriteLock self)
{
  EnterCriticalSection(&(self->cs_r));
  
  self->counter--;

  if (self->counter == 0)
  {
    LeaveCriticalSection(&(self->cs_w));
  }
  
  LeaveCriticalSection(&(self->cs_r));
}

//-----------------------------------------------------------------------------------

void ecreadwritelock_lockWrite (EcReadWriteLock self)
{
  EnterCriticalSection(&(self->cs_w));
}

//-----------------------------------------------------------------------------------

void ecreadwritelock_unlockWrite (EcReadWriteLock self)
{
  LeaveCriticalSection(&(self->cs_w));
}

//-----------------------------------------------------------------------------------

int ecreadwritelock_unlockReadAndTransformIfLast (EcReadWriteLock self)
{
  int ret;
  
  EnterCriticalSection(&(self->cs_r));
  
  self->counter--;
  
  ret = self->counter == 0;
  
  LeaveCriticalSection(&(self->cs_r));   
  
  return ret;
}

//-----------------------------------------------------------------------------------

int ecreadwritelock_lockReadAndTransformIfFirst (EcReadWriteLock self)
{
  int ret = FALSE;
  
  EnterCriticalSection(&(self->cs_r));

  if (self->counter == 0)
  {
    EnterCriticalSection(&(self->cs_w));
    ret = TRUE;
  }
  else
  {
    self->counter++;    
  }
  
  LeaveCriticalSection(&(self->cs_r));    
  
  return ret;  
}

//-----------------------------------------------------------------------------------

#endif
