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

#ifdef __GNUC__

#include "ecmutex.h"

#include <pthread.h>

//-----------------------------------------------------------------------------------

struct EcMutex_s {
  
  pthread_mutex_t mutex;
  
};

//-----------------------------------------------------------------------------------

EcMutex ecmutex_new()
{
  EcMutex self = ENTC_NEW(struct EcMutex_s);
  
  pthread_mutex_init(&(self->mutex), 0);
  
  return self;
}

//-----------------------------------------------------------------------------------

void ecmutex_delete(EcMutex* pself)
{
  EcMutex self = *pself;
  
  pthread_mutex_destroy(&(self->mutex));
  
  ENTC_DEL(pself, struct EcMutex_s);
}

//-----------------------------------------------------------------------------------

void ecmutex_lock(EcMutex self)
{
  pthread_mutex_lock(&(self->mutex));
}

//-----------------------------------------------------------------------------------

void ecmutex_unlock(EcMutex self)
{
  pthread_mutex_unlock(&(self->mutex));
}

//-----------------------------------------------------------------------------------

struct EcReadWriteLock_s
{
  
  pthread_mutex_t mutex_r;
  
  pthread_mutex_t mutex_w;
  
  int counter;
  
};

//-----------------------------------------------------------------------------------

EcReadWriteLock ecreadwritelock_new()
{
  EcReadWriteLock self = ENTC_NEW(struct EcReadWriteLock_s);
  
  pthread_mutex_init(&(self->mutex_r), 0);
  pthread_mutex_init(&(self->mutex_w), 0);
  
  self->counter = 0;
  
  return self;  
}

//-----------------------------------------------------------------------------------

void ecreadwritelock_delete(EcReadWriteLock* pself)
{
  EcReadWriteLock self = *pself;
  
  pthread_mutex_destroy(&(self->mutex_r));
  pthread_mutex_destroy(&(self->mutex_w));
  
  ENTC_DEL(pself, struct EcReadWriteLock_s);  
}

//-----------------------------------------------------------------------------------

void ecreadwritelock_lockRead(EcReadWriteLock self)
{
  pthread_mutex_lock(&(self->mutex_r));
  
  if (self->counter == 0)
  {
    pthread_mutex_lock(&(self->mutex_w));
  }
  
  self->counter++;
  
  pthread_mutex_unlock(&(self->mutex_r));
}

//-----------------------------------------------------------------------------------

void ecreadwritelock_unlockRead(EcReadWriteLock self)
{
  pthread_mutex_lock(&(self->mutex_r));
  
  self->counter--;

  if (self->counter == 0)
  {
    pthread_mutex_unlock(&(self->mutex_w));
  }
  
  pthread_mutex_unlock(&(self->mutex_r));  
}

//-----------------------------------------------------------------------------------

void ecreadwritelock_lockWrite(EcReadWriteLock self)
{
  pthread_mutex_lock(&(self->mutex_w));
}

//-----------------------------------------------------------------------------------

void ecreadwritelock_unlockWrite(EcReadWriteLock self)
{
  pthread_mutex_unlock(&(self->mutex_w));  
}

//-----------------------------------------------------------------------------------

#endif
