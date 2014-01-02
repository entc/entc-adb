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

#ifdef __APPLE_CC__

#include "ecevents.h"

#include "ecthread.h"
#include "ecfile.h"

//#ifdef TEST

#include "ecmutex.h"

#include <sys/types.h>
#include <sys/time.h>
#include <sys/event.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>

#include <memory.h>

struct EcEventContext_s
{
  
  EcList queues;
  
};


struct EcEventQueue_s
{
  
  int kq;
  
  struct kevent kev_set[10];
  
  int idents[10];
  
  int pos;
  
  EcListNode node;
  
};

//------------------------------------------------------------------------------------------------------------

EcEventContext ece_context_new()
{
  EcEventContext self = ENTC_NEW(struct EcEventContext_s);

  self->queues = eclist_new ();
  
  return self;
}

//------------------------------------------------------------------------------------------------------------

void ece_context_delete(EcEventContext* ptr)
{
  EcEventContext self = *ptr;
  
  eclist_delete (&(self->queues));
    
  ENTC_DEL(ptr, struct EcEventContext_s);
}

//------------------------------------------------------------------------------------------------------------

int ece_context_waitforTermination (EcEventContext self, uint_t timeout)
{
  // empty queue just waiting for abort event
  EcEventQueue queue = ece_queue_new (self);
  
  int rt = ece_queue_wait (queue, timeout, NULL);
  // cleanup
  ece_queue_delete (&queue);
  
  return rt;
}

//------------------------------------------------------------------------------------------------------------

void ece_context_triggerTermination (EcEventContext self)
{
  EcListNode node;
  for (node = eclist_first(self->queues); node != eclist_end(self->queues); node = eclist_next(node))
  {
    // trigger for all queue the abort event
    ece_queue_set (eclist_data(node), 0);
  }
}

//------------------------------------------------------------------------------------------------------------

EcEventQueue ece_queue_new (EcEventContext ec)
{
  EcEventQueue self = ENTC_NEW(struct EcEventQueue_s);
  
  self->kq = kqueue();
  self->pos = 0;
  {
    // set event as user event
    self->idents[self->pos] = 0;
    EV_SET( &(self->kev_set[self->pos]), 0, EVFILT_USER, EV_ADD | EV_ENABLE, 0, 0, NULL);
    self->pos++;
  }  
  
  self->node = eclist_append(ec->queues, self);
    
  return self;
}

//------------------------------------------------------------------------------------------------------------

void ece_queue_delete (EcEventQueue* sptr)
{
  EcEventQueue self = *sptr;
  
  eclist_erase(self->node);
  
  close(self->kq);
  
  ENTC_DEL(sptr, struct EcEventQueue_s);
}

//------------------------------------------------------------------------------------------------------------

void ece_queue_add (EcEventQueue self, EcHandle handle, int type)
{
  switch (type)
  {
    case ENTC_EVENTTYPE_READ:
    {
      self->idents[self->pos] = handle;
      EV_SET( &(self->kev_set[self->pos]), handle, EVFILT_READ, EV_ADD | EV_ENABLE | EV_CLEAR, 0, 0, NULL); 
      self->pos++;      
    }
    break;
    case ENTC_EVENTTYPE_WRITE:
    {
      self->idents[self->pos] = handle;
      EV_SET( &(self->kev_set[self->pos]), handle, EVFILT_WRITE, EV_ADD | EV_ENABLE | EV_CLEAR, 0, 0, NULL);       
      self->pos++;      
    }
    break;
    case ENTC_EVENTTYPE_USER:
    {
      self->idents[self->pos] = handle;
      EV_SET( &(self->kev_set[self->pos]), handle, EVFILT_USER, EV_ADD | EV_ENABLE | EV_CLEAR, 0, 0, NULL);   
      self->pos++;      
    }
    break;
  }
}

//------------------------------------------------------------------------------------------------------------

int ece_queue_wait (EcEventQueue self, uint_t timeout, EcLogger logger)
{
  int rt = 0;
  // register events
  {
    int res = kevent(self->kq, self->kev_set, self->pos, NULL, 0, NULL);
    if (res == -1)
    {
      eclogger_log (logger, LL_ERROR, "CORE", "{queue} register events failed");
      return -1;
    }
  }
  
  while (TRUE)
  {
    struct kevent kev_ret;
    // blocks until we got something
    int res = kevent(self->kq, NULL, 0, &kev_ret, 1, NULL);
    if( res == -1 )
    {
      if( errno == EINTR )
      {
        continue;
      }
      else
      {
        //eclogger_logerrno(logger, LOGMSG_ERROR, "CORE", "error on kevent");
        // some error       
      }
    }
    else if( res == 0 )
    {
      eclogger_log (logger, LL_TRACE, "CORE", "timeout on kevent");
      rt = ENTC_EVENT_TIMEOUT;
    }
    else if( (kev_ret.ident == 0) && (kev_ret.filter == EVFILT_USER) )
    {
      eclogger_log (logger, LL_TRACE, "CORE", "abort in eventcontext");
      // abort !!!!
      rt = ENTC_EVENT_ABORT;
    } 
    else
    {
      int i;
      // check all kev sets
      for (i = 0; i < self->pos; i++)
      {
        if (kev_ret.ident == self->idents[i])
        {
          //eclogger_logformat (logger, LOGMSG_DEBUG, "CORE", "got event with ident '%i'", kev_ret.ident);
          return i + ENTC_EVENT_ABORT + 1;
        }
      }
      eclogger_log (logger, LL_ERROR, "CORE", "{queue} unknown event found");
      rt = ENTC_EVENT_UNKNOWN;
    } 
    break;
  }  
  return rt;
}

//------------------------------------------------------------------------------------------------------------

EcHandle ece_queue_gen (EcEventQueue self)
{
  EcHandle handle = -self->pos;
  ece_queue_add (self, handle, ENTC_EVENTTYPE_USER);
  return handle;
}

//------------------------------------------------------------------------------------------------------------

void ece_queue_set (EcEventQueue self, EcHandle handle)
{
  struct kevent kev;
  memset(&kev, 0x0, sizeof(struct kevent));
  
  EV_SET( &kev, handle, EVFILT_USER, 0, NOTE_TRIGGER, 0, NULL);
  kevent(self->kq, &kev, 1, NULL, 0, NULL);
}

//------------------------------------------------------------------------------------------------------------

void ece_queue_del (EcEventQueue self, EcHandle* phandle)
{
  //
}

//------------------------------------------------------------------------------------------------------------

#endif
#ifdef __APPLE_CC__

typedef struct
{
  
  events_callback_fct onChange;
  
  void* onChangePtr;
  
  events_callback_fct onDelete;
  
  void* onDeletePtr;
  
  int fflags;
  
} EcEventFilesData;



struct EcEventFiles_s
{
  
  EcThread thread;
  
  int kqueue;
  
  int abortfd;
  
  int32_t size;
  
  EcEventFilesData* matrix;
  
  /* reference */
  EcLogger logger;
  
};



int ece_files_nextEvent(EcEventFiles self)
{
  struct kevent event;
  
  memset(&event, 0, sizeof(struct kevent));
  
  int res = kevent(self->kqueue, NULL, 0, &event, 1, NULL);
  
  eclogger_log(self->logger, LL_TRACE, "CORE", "{events} Received event");
  
  if( res == -1 )
  {
    return TRUE;  
  }
  
  if( res == 0 )
  {
    return TRUE;  
  }
  
#ifdef EVFILT_USER  
  if( (event.ident == self->abortfd) && (event.filter == EVFILT_USER) )
  {
    eclogger_log(self->logger, LL_TRACE, "CORE", "{events} Received abort event");
    
    return FALSE;
  }
#endif
  
  EcEventFilesData* data = &(self->matrix[event.ident]);
  
  if( event.fflags & NOTE_DELETE )
  {
    if( data->onDelete )
    {
      data->onDelete( data->onDeletePtr );
    }
  } 
  
  
  if( event.fflags & NOTE_WRITE )
  {
    eclogger_log(self->logger, LL_TRACE, "CORE", "{events} Received 'WRITE' event");
  }

  if( event.fflags & NOTE_EXTEND )
  {
    eclogger_log(self->logger, LL_TRACE, "CORE", "{events} Received 'EXTEND' event");
  }

  if( event.fflags & NOTE_ATTRIB )
  {
    eclogger_log(self->logger, LL_TRACE, "CORE", "{events} Received 'ATTRIB' event");
  }
   
  
  if( event.fflags & data->fflags )
  {        
    if( data->onChange )
    {
      data->onChange( data->onChangePtr );
    }
  }
  
  return TRUE;  
}

/*------------------------------------------------------------------------*/

int ecevents_thread(void* a)
{
  return ece_files_nextEvent(a);
}

/*------------------------------------------------------------------------*/

int ece_files_registerEvent (int kqueue, EcLogger logger, int filter, int fflags, int fd)
{
  /* variables */
  struct kevent kev;
  struct timespec nullts = { 0, 0 };
  int res;
  
  EV_SET( &kev, fd, filter, EV_ADD | EV_ENABLE | EV_CLEAR, fflags, 0, NULL);
  
  res = kevent(kqueue, &kev, 1, NULL, 0, &nullts);
  
  if( res < 0 )
  {
    eclogger_logerrno(logger, LL_FATAL, "CORE", "{events} Can't register kevent");
    
    return -1;  
  }
  else
  {
    return kev.ident;
  }
}

/*------------------------------------------------------------------------*/

#ifdef EVFILT_USER  
void ece_files_triggerEvent (int kqueue, EcLogger logger, int filter)
{
  /* variables */
  struct kevent kev;
  struct timespec nullts = { 0, 0 };
  int res;
  
  EV_SET( &kev, 0, filter, 0, NOTE_TRIGGER, 0, NULL);
  
  res = kevent(kqueue, &kev, 1, NULL, 0, &nullts);
  
  if( res < 0 )
  {
    eclogger_logerrno(logger, LL_ERROR, "CORE", "{events} Can't trigger kevent");
  }
}
#endif

/*------------------------------------------------------------------------*/

EcEventFiles ece_files_new (EcLogger logger)
{
  EcEventFiles self = ENTC_NEW (struct EcEventFiles_s);
  
  self->logger = logger;
  /* init kqueue */
  self->kqueue = kqueue();
#ifdef EVFILT_USER
  self->abortfd = ece_files_registerEvent (self->kqueue, logger, EVFILT_USER, 0, 0);
#else
  self->abortfd = 0;
#endif
  
  /* create a pool of matrix */
  self->size = 100;
  self->matrix = ENTC_MALLOC( sizeof(EcEventFilesData) * self->size );
  
  memset(self->matrix, 0, sizeof(EcEventFilesData) * self->size );
  
  self->thread = ecthread_new();
  
  ecthread_start(self->thread, ecevents_thread, (void*)self);
  
  return self;
}

/*------------------------------------------------------------------------*/

void ece_files_delete (EcEventFiles* ptr)
{
  EcEventFiles self = *ptr;
  
#ifdef EVFILT_USER  
  /* trigger the abort event */
  ece_files_triggerEvent (self->kqueue, self->logger, EVFILT_USER);
#endif
  
  /* wait until the thread returns */
  ecthread_join(self->thread);
  
  ecthread_delete(&(self->thread));
  
  close(self->kqueue);
  
  memset(self->matrix, 0, sizeof(EcEventFilesData) * self->size );
  
  ENTC_FREE( self->matrix );
  
  ENTC_DEL( ptr, struct EcEventFiles_s );
}

/*------------------------------------------------------------------------*/

void ece_files_resize (EcEventFiles self, int32_t resize)
{
  int32_t old_size = sizeof(EcEventFilesData) * self->size;
  int32_t new_size = sizeof(EcEventFilesData) * resize;
  /* allocate new buffer */
  EcEventFilesData* new_matrix = malloc( new_size );
  /* copy */
  memcpy(new_matrix, self->matrix, old_size);
  /* set */
  memset(new_matrix + old_size, 0, new_size - old_size );
  
  /* delete old matrix */
  memset(self->matrix, 0, old_size );
  ENTC_FREE( self->matrix );
  
  self->matrix = new_matrix;
  self->size = resize;
}

/*------------------------------------------------------------------------*/

void ece_files_register(EcEventFiles self, int fd, int filter, int fflags, events_callback_fct onChange, void* onChangePtr, events_callback_fct onDelete, void* onDeletePtr)
{
  int ident = ece_files_registerEvent (self->kqueue, self->logger, filter, fflags | NOTE_DELETE, fd);
  
  if( ident > 0 )
  {
    if( ident > self->size )
    {
      ece_files_resize( self, ident + self->size );
    }
    
    EcEventFilesData* data = &(self->matrix[ident]);
    
    data->onChange = onChange;
    data->onChangePtr = onChangePtr;
    data->onDelete = onDelete;
    data->onDeletePtr = onDeletePtr;
    data->fflags = fflags;
  }
}

/*------------------------------------------------------------------------*/
  
#endif
