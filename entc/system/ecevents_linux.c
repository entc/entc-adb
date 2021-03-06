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

#ifdef __linux__

#include "ecevents.h"

#include <utils/eclogger.h>

#include "ecthread.h"
#include "ecfile.h"
#include <errno.h>
#include <fcntl.h>
#include <stdint.h>
#include <sys/eventfd.h>

struct EcEventContext_s
{
  
  //int pipe[2];
  
  int efd;
  
};

//------------------------------------------------------------------------------------------------------------

void reactOnAbort (int rfd, int wfd)
{
  char buffer[100];
  int res = read(rfd, buffer, 100);
  if (res <= 0)
  {
    // some error happens
    return;
  }
  // write back
  res = write(wfd, buffer, res); 
}

//------------------------------------------------------------------------------------------------------------

void ece_sleep (unsigned long milliseconds)
{
  usleep (milliseconds * 1000);
}

//------------------------------------------------------------------------------------------------------------

void ece_set_event (int fd)
{
  uint64_t u = 1;
  int s = write(fd, &u, sizeof(uint64_t));
  if (s != sizeof(uint64_t))
  {
    eclogger_fmt (LL_ERROR, "ENTC", "event", "can't trigger");        
  }  
}

//------------------------------------------------------------------------------------------------------------

void ece_reset_event (int fd)
{
  char buffer [100];
  int res = 100;
  
  // try to read all bytes
  while (res == 100)
  {
    res = read (fd, buffer, 100);  
  }  
}

//------------------------------------------------------------------------------------------------------------

EcEventContext ece_context_new (void)
{
  EcEventContext self = ENTC_NEW(struct EcEventContext_s);
  
  self->efd = eventfd (0, 0);
  
  return self;
  /*
  int lp[2];
  
  // 0 = Read-End; 1 = WriteEnd
  pipe(lp);

  // make the both ends noneblocking  
  if (fcntl(lp[0], F_SETFL, fcntl( lp[0], F_GETFL, 0 ) | O_NONBLOCK ) < 0)
  {
    return NULL;
  }  
  if (fcntl(lp[1], F_SETFL, fcntl( lp[1], F_GETFL, 0 ) | O_NONBLOCK ) < 0)
  {
    return NULL;
  }

  {
    EcEventContext self = ENTC_NEW(struct EcEventContext_s);
  
    self->pipe[0] = lp[0];
    self->pipe[1] = lp[1];
  
    return self;
  }
  */
}

//------------------------------------------------------------------------------------------------------------

void ece_context_delete (EcEventContext* pself)
{
  EcEventContext self = *pself;
  
  /*
  if( self->pipe[1] > 0 )
  {
    close(self->pipe[0]);
    close(self->pipe[1]);
  }
  */
  
  close (self->efd);
  
  ENTC_DEL(pself, struct EcEventContext_s);
}

//------------------------------------------------------------------------------------------------------------

int ece_context_wait (EcEventContext self, EcHandle handle, uint_t timeout, int type)
{
  int retval;
  
  fd_set fdset;

  while (TRUE)
  {
    FD_ZERO(&fdset);
    FD_SET(self->efd, &fdset);
    FD_SET(handle, &fdset);      

    if (timeout == ENTC_INFINITE)
    {
      retval = select (ENTC_MAX(handle + 1, self->efd + 1), &fdset, NULL, NULL, NULL);      
    }
    else
    {
      struct timeval tv;
      tv.tv_sec = timeout / 1000;
      tv.tv_usec = (timeout % 1000) * 1000;
      
      retval = select (ENTC_MAX(handle + 1, self->efd + 1), &fdset, NULL, NULL, &tv);
    }

    if (retval == -1)
    {
      if (errno == EINTR)
      {
        continue;      
      }
    }
    else if (retval)
    {
      if( FD_ISSET(self->efd, &fdset) )
      {            
        return ENTC_EVENT_ABORT;
      }
      if( FD_ISSET(handle, &fdset) )
      {            
        return handle;
      }
      return ENTC_EVENT_ERROR;
    }
    else
    {
      return ENTC_EVENT_TIMEOUT;
    }   
  }  
}

//------------------------------------------------------------------------------------------------------------

int ece_context_waitforAbort (EcEventContext self, uint_t timeout)
{
  struct timeval tv;
  int retval;
  int rt = -1;
  
  fd_set fdset;

  while (TRUE)
  {
    FD_ZERO(&fdset);
    FD_SET(self->efd, &fdset);     

    if (timeout == ENTC_INFINITE)
    {
      retval = select (self->efd + 1, &fdset, NULL, NULL, NULL);      
    }
    else
    {
      tv.tv_sec = timeout / 1000;
      tv.tv_usec = (timeout % 1000) * 1000;
      
      retval = select (self->efd + 1, &fdset, NULL, NULL, &tv);
    }

    if (retval == -1)
    {
      if (errno == EINTR)
      {
        continue;      
      }
    }
    else if (retval)
    {
      if( FD_ISSET(self->efd, &fdset) )
      {            
        rt = ENTC_EVENT_ABORT;
      }
      else
      {
        rt = ENTC_EVENT_ERROR;
      }    
    }
    else
    {
      rt = ENTC_EVENT_TIMEOUT;
    }
    break;    
  }
  return rt; 
}

/*------------------------------------------------------------------------*/

void ece_context_setAbort (EcEventContext self)
{
  /*
  if( self->pipe[1] > 0 )
  {
    write(self->pipe[1], "\n", 1);    
  } 
  */

  ece_set_event (self->efd);
}

//------------------------------------------------------------------------------------------------------------

void ece_context_resetAbort (EcEventContext self)
{
  ece_reset_event (self->efd);
}

//------------------------------------------------------------------------------------------------------------

struct EcEventQueue_s
{
  
  EcEventContext ec;
  
  // make it thread safe
  EcMutex mutex;
  
  fd_set fdset_r;
  fd_set fdset_w;
  
  EcHandle intrfd;
  
  void* ptrset [FD_SETSIZE];
  char userhdl [FD_SETSIZE];
  
  ece_list_ondel_fct fct;

};

//------------------------------------------------------------------------------------------------------------

EcEventQueue ece_list_create (EcEventContext ec, ece_list_ondel_fct fct)
{
  EcEventQueue self = ENTC_NEW(struct EcEventQueue_s);

  self->ec = ec;
  self->mutex = ecmutex_new ();
  
  self->fct = fct;
   
  FD_ZERO (&(self->fdset_r));
  FD_ZERO (&(self->fdset_w));
  
  FD_SET (ec->efd, &(self->fdset_r));
  
  memset (self->ptrset, 0x00, sizeof(self->ptrset));
  memset (self->userhdl, 0x00, sizeof(self->userhdl));

  self->intrfd = eventfd (0, 0);
  ece_list_add (self, self->intrfd, ENTC_EVENTTYPE_USER, NULL);
  
  return self;    
}

//------------------------------------------------------------------------------------------------------------

void ece_list_destroy (EcEventQueue* pself)
{
  EcEventQueue self = *pself;
  
  int i;
  for (i = 0; i < FD_SETSIZE; i++)
  {
    void* ptr = self->ptrset [i];
    if (isAssigned (ptr) && isAssigned (self->fct))
    {
      self->fct (&ptr);
    }  
  }
    
  ecmutex_delete (&(self->mutex));
    
  ENTC_DEL (pself, struct EcEventQueue_s);    
}

//------------------------------------------------------------------------------------------------------------

void ece_list_set_ts (EcEventQueue self, EcHandle handle)
{
  if ((handle < FD_SETSIZE) && (self->userhdl [handle] == 0x42))
  {
    ece_set_event (handle);    
  }
}

//------------------------------------------------------------------------------------------------------------

int ece_list_add (EcEventQueue self, EcHandle handle, int type, void* ptr)
{
  int ret = TRUE;

  ece_set_event (self->intrfd);
    
  ecmutex_lock (self->mutex);
  
  if (handle < FD_SETSIZE)
  {
    if (type == ENTC_EVENTTYPE_WRITE)
    {
      FD_SET (handle, &(self->fdset_w));
      self->userhdl [handle] = 0x00;
    }
    else if (type == ENTC_EVENTTYPE_USER)
    {
      FD_SET (handle, &(self->fdset_r));    
      self->userhdl [handle] = 0x42;
    }
    else
    {
      FD_SET (handle, &(self->fdset_r));    
      self->userhdl [handle] = 0x00;
    }
    
    self->ptrset [handle] = ptr;
    
  }
  else
  {
    ret = FALSE; 
  }
  
  ecmutex_unlock (self->mutex);
  
  ece_reset_event (self->intrfd);
  
  return ret;
}

//------------------------------------------------------------------------------------------------------------

int ece_list_del (EcEventQueue self, EcHandle handle)
{
  ece_set_event (self->intrfd);

  ecmutex_lock (self->mutex);
  
  FD_CLR (handle, &(self->fdset_w));
  FD_CLR (handle, &(self->fdset_r));  
  
  void* ptr = self->ptrset [handle];
  if (isAssigned (ptr) && isAssigned (self->fct))
  {
    self->fct (&ptr);
    self->ptrset [handle] = NULL;
  }
  
  ecmutex_unlock (self->mutex);
  
  ece_reset_event (self->intrfd);
  
  return TRUE;
}

//------------------------------------------------------------------------------------------------------------

void ece_list_sortout (EcEventQueue self, ece_list_sort_out_fct callback, void* ptr)
{
  ecmutex_lock (self->mutex);

  int i;
  for (i = 0; i < FD_SETSIZE; i++)
  {
    void* obj = self->ptrset [i];
    
    // if true this entry must be removed
    if (isAssigned (obj) && callback (obj, ptr))
    {
      if (self->fct)
      {
        // call callback
        self->fct (&ptr);
      }

      self->ptrset [i] = NULL;
      
      FD_CLR (i, &(self->fdset_w));
      FD_CLR (i, &(self->fdset_r));  
    }
    
  }
  
  ecmutex_unlock (self->mutex);
}

//------------------------------------------------------------------------------------------------------------

int ece_list_select (EcEventQueue self, uint_t timeout, void** pptr)
{
  fd_set rfdset;
  fd_set wfdset;
  
  int retval, i;
  
  // make a local copy of the fdsets
  memcpy (&rfdset, &(self->fdset_r), sizeof (fd_set));
  memcpy (&wfdset, &(self->fdset_w), sizeof (fd_set));
           
  if (timeout == ENTC_INFINITE)
  {
    // eclogger_msg (LL_TRACE, "ENTC", "events", "try to wait for select");
    
    retval = select (FD_SETSIZE, &rfdset, &wfdset, NULL, NULL);      
  }
  else 
  {
    struct timeval tv;
    tv.tv_sec = timeout / 1000;
    tv.tv_usec = (timeout % 1000) * 1000;
    
    // eclogger_msg (LL_TRACE, "ENTC", "events", "try to wait for select (with timeout)");
    
    retval = select (FD_SETSIZE, &rfdset, &wfdset, NULL, &tv);
  }
    
  // eclogger_fmt (LL_TRACE, "ENTC", "events", "event triggered (%i)", retval);
  
  if (retval == -1)
  {
    if (errno == EINTR)
    {
      return ENTC_EVENT_INTERNAL;      
    }
    
    return ENTC_EVENT_ERROR;
  }
  else if (retval)
  {
    if (FD_ISSET (self->ec->efd, &rfdset))
    {
      eclogger_msg (LL_TRACE, "ENTC", "events", "got event (ABORT)");
           
      uint64_t s = read (self->ec->efd, &s, sizeof(uint64_t));
      if (s != sizeof(uint64_t))
      {
	eclogger_msg (LL_ERROR, "ENTC", "events", "read of user event returned wrong data");
      }
      
      uint64_t u = 1;
      s = write(self->ec->efd, &u, sizeof(uint64_t));
      if (s != sizeof(uint64_t))
      {
	eclogger_fmt (LL_ERROR, "ENTC", "event", "can't trigger");        
      }
      
      //eclogger_msg (LL_TRACE, "ENTC", "events", "return (ABORT)");
      
      return ENTC_EVENT_ABORT;
    }
    
    // internal interupt, is triggered by changes on the fdsets
    if (FD_ISSET (self->intrfd, &rfdset))
    { 
      //eclogger_msg (LL_TRACE, "ENTC", "events", "got event (INTR)");
      
      return ENTC_EVENT_INTERNAL;
    }

    for (i = 0; i < FD_SETSIZE; i++)
    {
      if (FD_ISSET (i, &rfdset) || FD_ISSET (i, &wfdset))
      {  
	if (isAssigned (pptr))
	{
	  *pptr = self->ptrset [i];
	}
	// user defined handle needs to read data
	// otherwise this event is triggered forever
	if (self->userhdl [i] == 0x42)
	{
	  //eclogger_msg (LL_TRACE, "ENTC", "events", "read (USER-EVENT)");
	  
	  uint64_t s = read (i, &s, sizeof(uint64_t));
	  if (s != sizeof(uint64_t))
	  {
	    i = ENTC_EVENT_ERROR; 
	  }  
	}
	
	//eclogger_msg (LL_TRACE, "ENTC", "events", "return (EVENT)");
	
	return i;
      }
    }
    return ENTC_EVENT_ERROR;
  }
  else
  {
    return ENTC_EVENT_TIMEOUT;
  }  
}

//------------------------------------------------------------------------------------------------------------

int ece_list_wait (EcEventQueue self, uint_t timeout, void** pptr)
{
  int retval = ENTC_EVENT_INTERNAL;
  
  while (retval == ENTC_EVENT_INTERNAL)
  {
    ecmutex_lock (self->mutex);
    
    retval = ece_list_select (self, timeout, pptr);
    
    ecmutex_unlock (self->mutex);
  }
  
  return retval;
}

//------------------------------------------------------------------------------------------------------------

EcHandle ece_list_handle (EcEventQueue self, void* ptr)
{
  EcHandle handle = eventfd (0, 0);
  if (handle > 0)
  {
    if (ece_list_add (self, handle, ENTC_EVENTTYPE_USER, ptr))
    {
      return handle;
    }
    else
    {
      close (handle);
    }
  }
  return 0;
}

//------------------------------------------------------------------------------------------------------------

void ece_list_set (EcEventQueue self, EcHandle handle)
{
  //ecmutex_lock (self->mutex);
   
  ece_list_set_ts (self, handle);
  
  //ecmutex_unlock (self->mutex);
}

//------------------------------------------------------------------------------------------------------------

int ece_list_size (EcEventQueue self)
{
  int i;
  int cnt = 0;
  
  ecmutex_lock (self->mutex);
  
  for (i = 0; i < FD_SETSIZE; i++)
  {
    void* obj = self->ptrset [i];

    if (obj)
    {
      cnt++;
    }
  }
  
  ecmutex_unlock (self->mutex); 
  
  return cnt;
}

//------------------------------------------------------------------------------------------------------------

#endif
#ifdef __linux__

#include <sys/inotify.h>

typedef struct
{
  
  events_callback_fct onChange;
  
  void* onChangePtr;
  
  events_callback_fct onDelete;
  
  void* onDeletePtr;
  
  // needed if file was replaced
  EcString filename;
  
} EcEventFilesData;


struct EcEventFiles_s
{
  
  EcEventContext econtext;
  
  EcThread thread;
  
  int notifd;
  
  int32_t size;
  
  EcEventFilesData matrix[FD_SETSIZE];
  
};
#define EVENT_SIZE ( sizeof (struct inotify_event) )
#define EVENT_BUF_LEN ( 1024 * (EVENT_SIZE + 16) )

/*------------------------------------------------------------------------*/

void ece_files_nextEvent2 (EcEventFiles self, struct inotify_event* pevent)
{
  EcEventFilesData* data;
  int ident = pevent->wd;
  
  if( (ident < 0) && (ident > self->size) )
  {
    eclogger_fmt (LL_ERROR, "ENTC", "inotify", "ident outside range", ident);
    /* ignore error */
    return;
  }
  
  data = &(self->matrix[ident]);
  
  if (pevent->mask & IN_IGNORED)
  {
    //eclogger_fmt (LL_TRACE, "ENTC", "inotify", "ident %i was replaced", ident);
    
    if (ecstr_valid(data->filename))
    {
      int new_ident = inotify_add_watch(self->notifd, data->filename, IN_ALL_EVENTS);
      
      if( new_ident < 0 )
      {
        eclogger_errno (LL_ERROR, "ENTC", "inotify", "can't register event for inotify");
        return;
      }
      
      memcpy(&(self->matrix[new_ident]), data, sizeof(EcEventFilesData));
    }
    else
    {
      eclogger_msg (LL_ERROR, "ENTC", "inotify", "can't reopen file: filename is empty");
    }
    return;
  }
  
  if( (pevent->mask & IN_DELETE) || (pevent->mask & IN_DELETE_SELF) || (pevent->mask & IN_MOVE_SELF) || (pevent->mask & IN_CREATE) )
  {
    //eclogger_fmt (LL_TRACE, "ENTC", "inotify", "remove event %i", ident);
    /* unregister event */
    inotify_rm_watch( self->notifd, ident );
    
    if( data->onDelete )
    {
      data->onDelete( data->onDeletePtr );
    }        
    return;
  }
  
  int changes = 
  (pevent->mask & IN_ATTRIB)
  || (pevent->mask & IN_CLOSE_WRITE)
  || (pevent->mask & IN_MODIFY)
  || (pevent->mask & IN_MOVED_FROM)
  || (pevent->mask & IN_MOVED_TO)
  || (pevent->mask & IN_MOVE_SELF);
  
  //eclogger_fmt (LL_TRACE, "ENTC", "inotify", "detect change [%u] A:%i C:%i M:%i F:%i T:%i S:%i O:%i R:%i", pevent->mask
  //             , pevent->mask & IN_ATTRIB, pevent->mask & IN_CLOSE_WRITE, pevent->mask & IN_MODIFY, pevent->mask & IN_MOVED_FROM,
  //               pevent->mask & IN_MOVED_TO, pevent->mask & IN_MOVE_SELF, pevent->mask & IN_OPEN, pevent->mask & IN_ACCESS
  //              );
  
  if ( changes )
  {
    if( data->onChange )
    {
      data->onChange( data->onChangePtr );
    }
  }
}

/*------------------------------------------------------------------------*/

int ece_files_nextEvent(EcEventFiles self)
{  
  int rt = FALSE;
    
  while (TRUE) 
  {
    int numRead;
    uint_t i = 0;
    char buf[EVENT_BUF_LEN] __attribute__ ((aligned(8)));
    char *p;
    
    // wait until some data received on one of the handles
    //eclogger_msg (LL_TRACE, "ENTC", "inotify", "wait for events");

    int res = ece_context_wait (self->econtext, self->notifd, ENTC_INFINITE, ENTC_EVENTTYPE_READ);
    // check the return
    if (res == ENTC_EVENT_ABORT)
    {
      // termination of the process
      break;
    }
    if (res == ENTC_EVENT_TIMEOUT)
    {
      // what to do ??
      break;
    }
    
    numRead = read(self->notifd, buf, EVENT_BUF_LEN);
    if (numRead < 0)
    {
      if (errno == EINTR)
      {
        /* need to reissue system call */  
        continue;
      }
      eclogger_errno (LL_ERROR, "ENTC", "inotify", "got error");
      break;
    }
    if (numRead == 0)
    {
      /* BUF_LEN to small ? */
      break;
    }
    
    //eclogger_fmt (LL_TRACE, "ENTC", "inotify", "got %i new events", numRead);
    
    for (p = buf; p < buf + numRead; )
    {
      struct inotify_event* pevent = (struct inotify_event *) p;
      
      ece_files_nextEvent2 (self, pevent);

      p += EVENT_SIZE + pevent->len;      
    }
    
    rt = TRUE;
    break;
  }
  
  //eclogger_fmt (LL_TRACE, "ENTC", "inotify", "ended with %i", rt);
  
  return rt;
}

/*------------------------------------------------------------------------*/

int ecevents_run(void* params)
{
  return ece_files_nextEvent(params);
}

/*------------------------------------------------------------------------*/

EcEventFiles ece_files_new ()
{
  EcEventFiles self = ENTC_NEW( struct EcEventFiles_s );
  
  self->econtext = ece_context_new ();
   
  self->notifd = inotify_init();
  
  memset(self->matrix, 0, sizeof(self->matrix));
  
  self->thread = ecthread_new();
  
  ecthread_start(self->thread, ecevents_run, self);
  
  return self;
}

/*------------------------------------------------------------------------*/

void ece_files_delete(EcEventFiles* ptr)
{
  int i;
  EcEventFiles self = *ptr;
  
  ece_context_setAbort (self->econtext);
  
  ecthread_join(self->thread);
  
  ecthread_delete(&(self->thread));
  
  ece_context_delete (&(self->econtext));
  
  // delete all strings
  for (i = 0; i < FD_SETSIZE; i++)
  {
    EcEventFilesData* data = &(self->matrix[i]);
    
    ecstr_delete(&(data->filename));
  }
  
  //inotify_done( self->notifd );
  
  ENTC_DEL( ptr, struct EcEventFiles_s );
}

/*------------------------------------------------------------------------*/

void ece_files_register (EcEventFiles self, const EcString filename, events_callback_fct onChange, void* onChangePtr, events_callback_fct onDelete, void* onDeletePtr)
{
  int ident = inotify_add_watch(self->notifd, filename, IN_ALL_EVENTS);
  
  if( ident < 0 )
  {
    eclogger_errno (LL_ERROR, "ENTC", "inotify", "can't register event for inotify");    
    return;
  }
    
  EcEventFilesData* data = &(self->matrix[ident]);
  
  data->onChange = onChange;
  data->onChangePtr = onChangePtr;
  data->onDelete = onDelete;
  data->onDeletePtr = onDeletePtr;
  ecstr_replace(&(data->filename), filename);
  
  //eclogger_fmt (LL_DEBUG, "ENTC", "inotify", "registered event at inotify on %i", ident);
  //eclogger_fmt (LL_TRACE, "ENTC", "inotify", " at '%s'", filename);
}

/*------------------------------------------------------------------------*/

#endif
