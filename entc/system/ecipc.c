/*
 * Copyright (c) 2010-2015 "Alexander Kalkhof" [email:entc@kalkhof.org]
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

#include "ecipc.h"

#ifdef __GNUC__

#include "utils/eclogger.h"

#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <errno.h>

struct EcShm_s
{
  
  int shid;
  
  int created;
  
};

//-----------------------------------------------------------------------------------

EcShm ecshm_create (EcAlloc alloc, uint32_t key, uint32_t size)
{
  int created = FALSE;
  // try to get shared memory
  int shmid = shmget (key, size, 0666);
  if (shmid < 0)
  {
    // try again if not exists
    if (errno == 2)
    {
      shmid = shmget(key, size, IPC_CREAT | 0666);
    }
    
    if (shmid < 0)
    {
      eclogger_errno (LL_ERROR, "ENTC", "shm", "can't get shared memory for %u", key);
      return NULL;
    }

    eclogger_msg (LL_TRACE, "ENTC", "shm", "created new memory");
    
    created = TRUE;
  }
  else
  {
    eclogger_msg (LL_TRACE, "ENTC", "shm", "use already existing memory");
  }
  
  EcShm self = ENTC_NEW (struct EcShm_s);
  
  self->shid = shmid;
  self->created = created;
  
  return self;
}

//-----------------------------------------------------------------------------------

void ecshm_destroy (EcShm* pself)
{
  
}

//-----------------------------------------------------------------------------------

void* ecshm_get (EcShm self)
{
  return shmat (self->shid, 0, 0);
}

//-----------------------------------------------------------------------------------

int ecshm_wasCreated (EcShm self)
{
  return self->created;
}

//-----------------------------------------------------------------------------------

struct EcSem_s
{
  
  int semid;
  
  int chan;
  
};

//-----------------------------------------------------------------------------------

EcSem ecsem_create (EcAlloc alloc, uint32_t key)
{
  // try to get semaphore
  int semid = semget (key, 1, 0666);
  if (semid < 0)
  {
    // try again if not exists
    if (errno == 2)
    {
      semid = semget (key, 1, IPC_CREAT | 0666);
    }
    
    if (semid < 0)
    {
      eclogger_errno (LL_ERROR, "ENTC", "shm", "can't get sempaphore for %u", key);
      return NULL;
    }
    
    eclogger_msg (LL_TRACE, "ENTC", "shm", "created new sempaphore");
  }
  else
  {
    eclogger_fmt (LL_TRACE, "ENTC", "shm", "use already existing sempaphore [%i] for key %i", semid, key);
  }
  
  EcSem self = ECMM_NEW (struct EcSem_s);
  
  if (self == NULL)
  {
    eclogger_fmt (LL_FATAL, "ENTC", "shm", "can't allocate memory for object");
    return NULL;
  }
  
  self->semid = semid;
  self->chan = 0;
  
  return self;  
}

//-----------------------------------------------------------------------------------

void ecsem_destroy (EcSem* pself)
{
  
}

//-----------------------------------------------------------------------------------

void ecsem_clear (EcSem self)
{
  int res = semctl(self->semid, 0, SETVAL, 0);
  if (res < 0)
  {
    eclogger_errno (LL_ERROR, "ENTC", "shm", "can't set operation"); 
  }   
}

//-----------------------------------------------------------------------------------

void ecsem_waitAndSet (EcSem self, int waitFor, int setVal)
{
  struct sembuf sops[2];
  
  sops[0].sem_num = 0;            // we only use one track 
  sops[0].sem_op = waitFor;       // wait for semaphore flag to become zero 
  sops[0].sem_flg = SEM_UNDO;     // take off semaphore asynchronous
  
  sops[1].sem_num = 0;            // Operate on semaphore 0 
  sops[1].sem_op = setVal;        // Increment value by one
  sops[1].sem_flg = SEM_UNDO;
  
  int res = semop (self->semid, sops, 2);
  if (res < 0)
  {
    eclogger_errno (LL_ERROR, "ENTC", "shm", "can't set operation"); 
    return;
  } 
  
  eclogger_fmt (LL_TRACE, "ENTC", "sem", "sem [%i] values %i", self->semid, 1); 
}

//-----------------------------------------------------------------------------------

int ecsem_tryDec (EcSem self)
{
  struct sembuf sops[2];

  sops[0].sem_num = 0;        // Operate on semaphore 0 
  sops[0].sem_op = 0;         // Increment value by one
  sops[0].sem_flg = SEM_UNDO | IPC_NOWAIT;

  int res = semop (self->semid, sops, 1);
  if (res < 0)
  {
    int h = errno;
    
    eclogger_errno (LL_ERROR, "ENTC", "shm", "can't set operation [%i]", res); 
    return h == EAGAIN;
  } 
  
  
  return FALSE;  
}

//-----------------------------------------------------------------------------------

void ecsem_wait (EcSem self, int waitFor)
{
  struct sembuf sops[2];
  
  sops[0].sem_num = 0;             // we only use one track 
  sops[0].sem_op = waitFor;        // wait for semaphore flag to become zero 
  sops[0].sem_flg = SEM_UNDO;      // take off semaphore asynchronous
  
  eclogger_fmt (LL_TRACE, "ENTC", "sem", "sem [%i] wait", self->semid); 

  int res = semop (self->semid, sops, 1);
  if (res < 0)
  {
    eclogger_errno (LL_ERROR, "ENTC", "shm", "can't set operation"); 
    return;
  }  
}
  
//-----------------------------------------------------------------------------------

void ecsem_set (EcSem self, int setVal)
{
  struct sembuf sops[1];
  
  sops[0].sem_num = 0;             // Operate on semaphore 0 
  sops[0].sem_op =  setVal;        // Increment value by one
  sops[0].sem_flg = SEM_UNDO;
  
  int res = semop (self->semid, sops, 1);
  if (res < 0)
  {
    eclogger_errno (LL_ERROR, "ENTC", "shm", "can't set operation"); 
    return;
  }  

  /*
  int val;
  sem_getvalue (self->semid, &val);

   */
  eclogger_fmt (LL_TRACE, "ENTC", "sem", "sem [%i] values %i", self->semid, setVal); 
}

//-----------------------------------------------------------------------------------

void ecsem_queue_wait (EcSem self)
{
  
}

//-----------------------------------------------------------------------------------

void ecsem_queue_send (EcSem self)
{
  
}

//-----------------------------------------------------------------------------------

#endif
