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

#endif
