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
#include "utils/eclogger.h"

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

//------------------------------------------------------------------------------------------------------------

void ece_sleep (unsigned long milliseconds)
{
  usleep (milliseconds * 1000);
}

//------------------------------------------------------------------------------------------------------------

void ece_kevent_addHandle (int kq, EcHandle handle, int flag, void* ptr, int clear)
{
  struct kevent kev;
  memset (&kev, 0x0, sizeof(struct kevent));
  
  if (clear)
  {
    EV_SET (&kev, handle, flag, EV_ADD | EV_ENABLE | EV_CLEAR, 0, 0, ptr); 
  }
  else
  {
    EV_SET (&kev, handle, flag, EV_ADD | EV_ENABLE, 0, 0, ptr);     
  }
  // add handle to kevent
  kevent (kq, &kev, 1, NULL, 0, NULL);
}

//------------------------------------------------------------------------------------------------------------

void ece_kevent_delHandle (int kq, EcHandle handle)
{
  struct kevent kev;
  struct kevent res;
  memset(&kev, 0x0, sizeof(struct kevent));
  memset(&res, 0x0, sizeof(struct kevent));
  
  EV_SET (&kev, handle, EVFILT_READ | EVFILT_WRITE | EVFILT_USER, EV_DISPATCH | EV_RECEIPT, 0, 0, NULL);
  kevent (kq, &kev, 1, &res, 1, NULL);  
  
  if (handle > 0)
  {
    close (handle);
  }
}

//------------------------------------------------------------------------------------------------------------

void ece_kevent_setHandle (int kq, EcHandle handle, int flag)
{
  struct kevent kev;
  memset (&kev, 0x0, sizeof(struct kevent));
  
  EV_SET( &kev, handle, flag, 0, NOTE_TRIGGER, 0, NULL);
  kevent (kq, &kev, 1, NULL, 0, NULL);
}

//------------------------------------------------------------------------------------------------------------

int ece_kevent_wait (int kq, struct kevent* event, uint_t timeout)
{
  int res;
  // blocks until we got something
  if (timeout == ENTC_INFINTE)
  {
    res = kevent (kq, NULL, 0, event, 1, NULL);
  }
  else
  {
    struct timespec tmout;
    
    tmout.tv_sec = timeout / 1000;
    tmout.tv_nsec = (timeout % 1000) * 1000;
    
    res = kevent (kq, NULL, 0, event, 1, &tmout);      
  }  
  return res;
}

//------------------------------------------------------------------------------------------------------------

struct EcEventContext_s
{

  EcMutex mutex;
  
  EcList lists;
  
};

//------------------------------------------------------------------------------------------------------------

EcEventContext ece_context_new ()
{
  EcEventContext self = ENTC_NEW(struct EcEventContext_s);
  
  self->mutex = ecmutex_new ();
  self->lists = eclist_new ();
  
  return self;  
}

//------------------------------------------------------------------------------------------------------------

void ece_context_delete (EcEventContext* pself)
{
  EcEventContext self = *pself;

  ecmutex_delete (&(self->mutex));
  eclist_delete (&(self->lists));
  
  ENTC_DEL(pself, struct EcEventContext_s);
}

//------------------------------------------------------------------------------------------------------------

int ece_context_wait (EcEventContext self, EcHandle handle, uint_t timeout, int type)
{
  EcEventQueue list = ece_list_create (self, NULL);
  
  ece_list_add (list, handle, type, NULL);
  
  int ret = ece_list_wait (list, timeout, NULL);
  
  ece_list_destroy (&list);
  
  return ret;
}

//------------------------------------------------------------------------------------------------------------

int ece_context_waitforTermination (EcEventContext self, uint_t timeout)
{
  EcEventQueue list = ece_list_create (self, NULL);
  
  int ret = ece_list_wait (list, timeout, NULL);
  
  ece_list_destroy (&list);
  
  return ret;
}

//------------------------------------------------------------------------------------------------------------

#define ECELIST_ABORT_HANDLENO -1
#define ECELIST_MAX_USEREVENTS 200

void ece_context_triggerTermination (EcEventContext self)
{
  EcListNode node;
    
  ecmutex_lock (self->mutex);

  for (node = eclist_first(self->lists); node != eclist_end(self->lists); node = eclist_next(node))
  {
    EcEventQueue list = eclist_data (node);
    // trigger termination in queue    
    ece_list_set (list, ECELIST_ABORT_HANDLENO);
  }

  ecmutex_unlock (self->mutex);
}

//------------------------------------------------------------------------------------------------------------

typedef struct
{

  int handle;
  
  void* ptr;

} EcEventData;

struct EcEventQueue_s
{

  // reference from context
  int kq;
    
  EcListNode ecnode;
  
  EcMutex ecmutex;
  
  ece_list_ondel_fct fct;

  char data [ECELIST_MAX_USEREVENTS];
  
  EcList ptrs;

};

//------------------------------------------------------------------------------------------------------------

EcEventQueue ece_list_create (EcEventContext ec, ece_list_ondel_fct fct)
{
  EcEventQueue self = ENTC_NEW(struct EcEventQueue_s);

  // create a new queue
  self->kq = kqueue ();
  
  self->ecmutex = ec->mutex;
  self->fct = fct;
  
  ecmutex_lock (self->ecmutex);

  self->ecnode = eclist_append(ec->lists, self);
  
  ecmutex_unlock (self->ecmutex);

  ece_kevent_addHandle (self->kq, ECELIST_ABORT_HANDLENO, EVFILT_USER, NULL, FALSE);
    
  memset (self->data, 0x0, sizeof(char) * 200);
  
  self->ptrs = eclist_new ();
  
  return self;
}

//------------------------------------------------------------------------------------------------------------

void ece_list_clear (EcEventQueue self)
{
  EcListCursor cursor; eclist_cursor (self->ptrs, &cursor);
  
  while (eclist_cnext (&cursor))
  {
    EcEventData* pdata = cursor.value;

    ece_kevent_delHandle (self->kq, pdata->handle);
    
    if (self->fct)
    {
      // call callback
      self->fct (&(pdata->ptr));
    }
    
    if (pdata->handle < ECELIST_ABORT_HANDLENO)
    {
      self->data [ECELIST_ABORT_HANDLENO - pdata->handle - 1] = 0;
    }
    
    ENTC_DEL (&pdata, EcEventData);
  }
  
  eclist_clear (self->ptrs);
}

//------------------------------------------------------------------------------------------------------------

void ece_list_destroy (EcEventQueue* sptr)
{
  EcEventQueue self = *sptr;

  ecmutex_lock (self->ecmutex);
  
  eclist_erase (self->ecnode);
  
  ecmutex_unlock (self->ecmutex);
  
  // cleanup
  ece_list_clear (self);
  eclist_delete(&(self->ptrs));
      
  close (self->kq);
  
  ENTC_DEL(sptr, struct EcEventQueue_s);
}

//------------------------------------------------------------------------------------------------------------

void ece_list_data_add (EcEventQueue self, EcHandle handle, void* ptr)
{
  if (isAssigned (ptr))
  {
    EcEventData* pdata = ENTC_NEW (EcEventData);
    
    pdata->handle = handle;
    pdata->ptr = ptr;
    
    eclist_append(self->ptrs, pdata);  
  }
}

//------------------------------------------------------------------------------------------------------------

void* ece_list_data_get (EcEventQueue self, EcHandle handle, int remove)
{
  EcListCursor cursor; eclist_cursor (self->ptrs, &cursor);
  
  while (eclist_cnext (&cursor))
  {
    EcEventData* pdata = cursor.value;
    
    if (pdata->handle == handle)
    {
      if (remove)
      {
        void* ptr = pdata->ptr;
        
        eclist_erase (cursor.node);
        
        ENTC_DEL (&pdata, EcEventData);
        
        return ptr;
      }
      else
      {
        return pdata->ptr;        
      }
    }
  }
  return NULL;
}

//------------------------------------------------------------------------------------------------------------

int ece_list_add (EcEventQueue self, EcHandle handle, int type, void* ptr)
{
  int flag = 0;
  int clr = FALSE;
  switch (type)
  {
    case ENTC_EVENTTYPE_READ:
    {
      flag = EVFILT_READ;
    }
    break;
    case ENTC_EVENTTYPE_WRITE:
    {
      flag = EVFILT_WRITE;
    }
    break;
    case ENTC_EVENTTYPE_USER:
    {
      flag = EVFILT_USER;
      clr = TRUE;
    }
    break;
  }

  ece_kevent_addHandle (self->kq, handle, flag, ptr, clr); 
  ece_list_data_add (self, handle, ptr);
  
  return TRUE;
}

//------------------------------------------------------------------------------------------------------------

int ece_list_wait (EcEventQueue self, uint_t timeout, void** pptr)
{
  while (TRUE)
  {
    struct kevent kev_ret;
    memset (&kev_ret, 0x0, sizeof(struct kevent));

    int res = ece_kevent_wait (self->kq, &kev_ret, timeout);
    if( res == -1 )
    {
      if( errno == EINTR )
      {
        continue;
      }
      else
      {
        if (isAssigned (pptr))
        { 
          *pptr = NULL;
        }
        //eclogger_logerrno(logger, LOGMSG_ERROR, "CORE", "error on kevent");
        // some error    
        return ENTC_EVENT_ERROR;
      }
    }
    else if (kev_ret.flags & EV_ERROR)
    {
      if (isAssigned (pptr))
      { 
        *pptr = NULL;
      }
      return ENTC_EVENT_ERROR;
    }
    else if( res == 0 )
    {
      if (isAssigned (pptr))
      { 
        *pptr = NULL;
      }
      //eclogger_log (logger, LL_TRACE, "CORE", "timeout on kevent");
      return ENTC_EVENT_TIMEOUT;
    }
    else if( (kev_ret.ident == ECELIST_ABORT_HANDLENO) && (kev_ret.filter == EVFILT_USER) )
    {
      if (isAssigned (pptr))
      { 
        *pptr = NULL;
      }
      //eclogger_log (logger, LL_TRACE, "CORE", "abort in eventcontext");      
      return ENTC_EVENT_ABORT;
    } 
    else
    {
      //eclogger_logformat (logger, LL_TRACE, "CORE", "got event with ident '%i' with udata %p", kev_ret.ident, kev_ret.udata);
      
      if (isAssigned (pptr))
      {
        *pptr = ece_list_data_get (self, kev_ret.ident, FALSE);
      }
      
      return kev_ret.ident;
    }  
  }
}

//------------------------------------------------------------------------------------------------------------

int ece_list_del (EcEventQueue self, EcHandle handle)
{
  ece_kevent_delHandle (self->kq, handle);

  if (self->fct)
  {
    void* ptr = ece_list_data_get (self, handle, TRUE);    
    
    // call callback
    self->fct (&ptr);
    
    ptr = NULL;
  }
  
  if (handle < ECELIST_ABORT_HANDLENO)
  {
    self->data [ECELIST_ABORT_HANDLENO - handle - 1] = 0;
  }
  
  return TRUE;
}

//------------------------------------------------------------------------------------------------------------

int ece_list_getNextHandle (EcEventQueue self)
{
  // find next free handle slot
  int i;
  for (i = 0; i < ECELIST_MAX_USEREVENTS; i++)
  {
    if (self->data [i] == 0)
    {
      return ECELIST_ABORT_HANDLENO - i - 1;
    }
  }
  return 0;
}

//------------------------------------------------------------------------------------------------------------

EcHandle ece_list_handle (EcEventQueue self, void* ptr)
{
  int handle = ece_list_getNextHandle (self);
  
  if (handle < 0)
  {
    self->data [ECELIST_ABORT_HANDLENO - handle - 1] = 1;
    ece_kevent_addHandle (self->kq, handle, EVFILT_USER, ptr, TRUE);
    
    ece_list_data_add (self, handle, ptr);
  }
  
  return handle;
}

//------------------------------------------------------------------------------------------------------------

void ece_list_set (EcEventQueue self, EcHandle handle)
{
  ece_kevent_setHandle (self->kq, handle, EVFILT_USER);
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
  
};



int ece_files_nextEvent(EcEventFiles self)
{
  struct kevent event;
  
  memset(&event, 0, sizeof(struct kevent));
  
  int res = kevent(self->kqueue, NULL, 0, &event, 1, NULL);
  
  eclogger_msg (LL_TRACE, "ENTC", "events", "Received event");
  
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
    eclogger_msg (LL_TRACE, "ENTC", "events", "Received abort event");
    
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
    eclogger_msg (LL_TRACE, "ENTC", "events", "Received 'WRITE' event");
  }

  if( event.fflags & NOTE_EXTEND )
  {
    eclogger_msg (LL_TRACE, "ENTC", "events", "Received 'EXTEND' event");
  }

  if( event.fflags & NOTE_ATTRIB )
  {
    eclogger_msg (LL_TRACE, "ENTC", "events", "Received 'ATTRIB' event");
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

int ece_files_registerEvent (int kqueue, int filter, int fflags, int fd)
{
  /* variables */
  struct kevent kev;
  struct timespec nullts = { 0, 0 };
  int res;
  
  EV_SET( &kev, fd, filter, EV_ADD | EV_ENABLE | EV_CLEAR, fflags, 0, NULL);
  
  res = kevent(kqueue, &kev, 1, NULL, 0, &nullts);
  
  if( res < 0 )
  {
    eclogger_errno (LL_FATAL, "ENTC", "events", "Can't register kevent");    
    return -1;  
  }
  else
  {
    return kev.ident;
  }
}

/*------------------------------------------------------------------------*/

#ifdef EVFILT_USER  
void ece_files_triggerEvent (int kqueue, int filter)
{
  /* variables */
  struct kevent kev;
  struct timespec nullts = { 0, 0 };
  int res;
  
  EV_SET( &kev, 0, filter, 0, NOTE_TRIGGER, 0, NULL);
  
  res = kevent(kqueue, &kev, 1, NULL, 0, &nullts);
  
  if( res < 0 )
  {
    eclogger_errno (LL_ERROR, "ENTC", "events", "Can't trigger kevent");    
  }
}
#endif

/*------------------------------------------------------------------------*/

EcEventFiles ece_files_new ()
{
  EcEventFiles self = ENTC_NEW (struct EcEventFiles_s);
  
  /* init kqueue */
  self->kqueue = kqueue();
#ifdef EVFILT_USER
  self->abortfd = ece_files_registerEvent (self->kqueue, EVFILT_USER, 0, 0);
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
  ece_files_triggerEvent (self->kqueue, EVFILT_USER);
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
  int ident = ece_files_registerEvent (self->kqueue, filter, fflags | NOTE_DELETE, fd);
  
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
