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

    if (timeout == ENTC_INFINTE)
    {
      retval = select (handle + 1, &fdset, NULL, NULL, NULL);      
    }
    else
    {
      struct timeval tv;
      tv.tv_sec = timeout / 1000;
      tv.tv_usec = (timeout % 1000) * 1000;
      
      retval = select (handle + 1, &fdset, NULL, NULL, &tv);
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

int ece_context_waitforTermination (EcEventContext self, uint_t timeout)
{
    struct timeval tv;
  int retval;
  int rt = -1;
  
  fd_set fdset;

  while (TRUE)
  {
    FD_ZERO(&fdset);
    FD_SET(self->efd, &fdset);     

    if (timeout == ENTC_INFINTE)
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

void ece_context_triggerTermination (EcEventContext self)
{
  /*
  if( self->pipe[1] > 0 )
  {
    write(self->pipe[1], "\n", 1);    
  } 
  */


  uint64_t u = 1;
  int s = write(self->efd, &u, sizeof(uint64_t));
  if (s != sizeof(uint64_t))
  {
    
  }
}

//------------------------------------------------------------------------------------------------------------

struct EcEventQueue_s
{
  
  EcEventContext ec;
  
  fd_set fdset_r;
  fd_set fdset_w;
  
  EcHandle intrfd;
  
  void* ptrset [FD_SETSIZE];
  
  ece_list_ondel_fct fct;

};

//------------------------------------------------------------------------------------------------------------

EcEventQueue ece_list_create (EcEventContext ec, ece_list_ondel_fct fct)
{
  EcEventQueue self = ENTC_NEW(struct EcEventQueue_s);

  self->ec = ec;
  self->fct = fct;
   
  FD_ZERO (&(self->fdset_r));
  FD_ZERO (&(self->fdset_w));
  
  FD_SET (ec->efd, &(self->fdset_r));
  
  memset (self->ptrset, NULL, sizeof(self->ptrset));
  
  self->intrfd = ece_list_handle (self, NULL);
  
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
    
  ENTC_DEL (pself, struct EcEventQueue_s);    
}

//------------------------------------------------------------------------------------------------------------

int ece_list_add (EcEventQueue self, EcHandle handle, int type, void* ptr)
{
  if (handle < FD_SETSIZE)
  {
    if (type == ENTC_EVENTTYPE_WRITE)
    {
      FD_SET (handle, &(self->fdset_w));
    }
    else
    {
      FD_SET (handle, &(self->fdset_r));    
    }
    
    self->ptrset [handle] = ptr;
    
    ece_list_set (self, self->intrfd);
    
    return TRUE;
  }
  else
  {
    return FALSE; 
  }
}

//------------------------------------------------------------------------------------------------------------

int ece_list_del (EcEventQueue self, EcHandle handle)
{
  FD_CLR (handle, &(self->fdset_w));
  FD_CLR (handle, &(self->fdset_r));
  
  ece_list_set (self, self->intrfd);
  
  void* ptr = self->ptrset [handle];
  if (isAssigned (ptr) && isAssigned (self->fct))
  {
    self->fct (&ptr);
    self->ptrset [handle] = NULL;
  }
  
  return TRUE;
}

//------------------------------------------------------------------------------------------------------------

int ece_list_wait (EcEventQueue self, uint_t timeout, void** pptr, EcLogger logger)
{
  int retval, i;
  
  fd_set rfdset;
  fd_set wfdset;
  
  
  while (TRUE)
  {
    memcpy (&rfdset, &(self->fdset_r), sizeof (fd_set));
    memcpy (&wfdset, &(self->fdset_w), sizeof (fd_set));
       
    if (timeout == ENTC_INFINTE)
    {
      retval = select (FD_SETSIZE, &rfdset, &wfdset, NULL, NULL);      
    }
    else 
    {
      struct timeval tv;
      tv.tv_sec = timeout / 1000;
      tv.tv_usec = (timeout % 1000) * 1000;
      
      //eclogger_logformat(logger, LOGMSG_DEBUG, "CORE", "{queue::pipe} wait for timeout sec:%lu usec:%lu maxfd:%i", tv.tv_sec, tv.tv_usec, self->maxfd);            
      
      retval = select (FD_SETSIZE, &rfdset, &wfdset, NULL, &tv);
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
      if (FD_ISSET (self->ec->efd, &rfdset))
      { 
	return ENTC_EVENT_ABORT;
      }
      if (FD_ISSET (self->intrfd, &rfdset))
      { 
	uint64_t s = read (self->intrfd, &s, sizeof(uint64_t));
        if (s != sizeof(uint64_t))
	{
	  return ENTC_EVENT_ERROR; 
	}
	continue;
      }
      for (i = 0; i < FD_SETSIZE; i++)
      {
	if (FD_ISSET (i, &rfdset) || FD_ISSET (i, &wfdset))
	{
	  if (isAssigned (pptr))
	  {
	    *pptr = self->ptrset [i];
	  }
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
}

//------------------------------------------------------------------------------------------------------------

EcHandle ece_list_handle (EcEventQueue self, void* ptr)
{
  EcHandle handle = eventfd (0, 0);
  if (ece_list_add (self, handle, ENTC_EVENTTYPE_USER, ptr))
  {
    return handle;
  }
  else
  {
    close (handle);
    return 0;
  }
}

//------------------------------------------------------------------------------------------------------------

void ece_list_set (EcEventQueue self, EcHandle handle)
{
  uint64_t u = 1;
  int s = write (handle, &u, sizeof(uint64_t));
  if (s != sizeof(uint64_t))
  {
    
  }
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
  
  EcEventFilesData* matrix;
  
  /* reference */
  EcLogger logger;
  
};
#define EVENT_SIZE ( sizeof (struct inotify_event) )
#define EVENT_BUF_LEN ( 1024 * (EVENT_SIZE + 16) )

/*------------------------------------------------------------------------*/

void ece_files_resize(EcEventFiles self, int32_t resize)
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
  free( self->matrix );
  
  self->matrix = new_matrix;
  self->size = resize;
}

/*------------------------------------------------------------------------*/

void ece_files_nextEvent2 (EcEventFiles self, struct inotify_event* pevent)
{
  EcEventFilesData* data;
  int ident = pevent->wd;
  
  if( (ident < 0) && (ident > self->size) )
  {
    eclogger_logformat(self->logger, LL_ERROR, "CORE", "{ece_files} ident outside range", ident);
    /* ignore error */
    return;
  }
  
  data = &(self->matrix[ident]);
  
  if (pevent->mask & IN_IGNORED)
  {
    eclogger_logformat(self->logger, LL_TRACE, "CORE", "{ece_files} ident %i was replaced", ident);
    
    if (ecstr_valid(data->filename))
    {
      int new_ident = inotify_add_watch(self->notifd, data->filename, IN_ALL_EVENTS);
      
      if( new_ident < 0 )
      {
        eclogger_logerrno(self->logger, LL_ERROR, "CORE", "{ece_files} Can't register event for inotify");
        return;
      }
      
      if( new_ident > self->size )
      {
        ece_files_resize( self, ident + self->size );
      }
      
      memcpy(&(self->matrix[new_ident]), data, sizeof(EcEventFilesData));
    }
    else
    {
      eclogger_log(self->logger, LL_ERROR, "CORE", "{ece_files} can't reopen file: filename is empty");
    }
    return;
  }
  
  if( (pevent->mask & IN_DELETE) || (pevent->mask & IN_DELETE_SELF) || (pevent->mask & IN_MOVE_SELF) || (pevent->mask & IN_CREATE) )
  {
    eclogger_logformat(self->logger, LL_TRACE, "CORE", "{ece_files} remove event %i", ident);
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
  
  eclogger_logformat(self->logger, LL_TRACE, "CORE", "{ece_files} detect change [%u] A:%i C:%i M:%i F:%i T:%i S:%i O:%i R:%i", pevent->mask
                     , pevent->mask & IN_ATTRIB, pevent->mask & IN_CLOSE_WRITE, pevent->mask & IN_MODIFY, pevent->mask & IN_MOVED_FROM,
                     pevent->mask & IN_MOVED_TO, pevent->mask & IN_MOVE_SELF, pevent->mask & IN_OPEN, pevent->mask & IN_ACCESS
                     );
  
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
    int len;
    uint_t i = 0;
    char buf[EVENT_BUF_LEN];

    // wait until some data received on one of the handles
    eclogger_log(self->logger, LL_TRACE, "CORE", "{ece_files} wait for events");

    int res = ece_context_wait (self->econtext, self->notifd, ENTC_INFINTE, ENTC_EVENTTYPE_READ);
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
    
    len = read(self->notifd, buf, EVENT_BUF_LEN);
    if (len < 0)
    {
      if (errno == EINTR)
      {
        /* need to reissue system call */  
        continue;
      }
      eclogger_logerrno(self->logger, LL_ERROR, "CORE", "{ece_files} got error");
      break;
    }
    if (len == 0)
    {
      /* BUF_LEN to small ? */
      break;
    }
    
    eclogger_logformat(self->logger, LL_TRACE, "CORE", "{ece_files} got %i new events", len);
    
    while (i < len)
    {    
      struct inotify_event* pevent = (struct inotify_event*)&buf[i];
      
      ece_files_nextEvent2 (self, pevent);
      
      i += EVENT_SIZE + pevent->len;      
    }
    
    rt = TRUE;
    break;
  }
  
  eclogger_logformat(self->logger, LL_TRACE, "CORE", "{ece_files} ended with %i", rt);
  
  return rt;
}

/*------------------------------------------------------------------------*/

int ecevents_run(void* params)
{
  return ece_files_nextEvent(params);
}

/*------------------------------------------------------------------------*/

EcEventFiles ece_files_new(EcLogger logger)
{
  EcEventFiles self = ENTC_NEW( struct EcEventFiles_s );
  
  self->econtext = ece_context_new ();
  
  self->logger = logger;
  
  self->notifd = inotify_init();
  
  /* create a pool of matrix */
  self->size = 100;
  self->matrix = malloc( sizeof(EcEventFilesData) * self->size );
  
  memset(self->matrix, 0, sizeof(EcEventFilesData) * self->size );
  
  self->thread = ecthread_new();
  
  ecthread_start(self->thread, ecevents_run, self);
  
  return self;
}

/*------------------------------------------------------------------------*/

void ece_files_delete(EcEventFiles* ptr)
{
  EcEventFiles self = *ptr;
  
  ece_context_triggerTermination (self->econtext);
  
  ecthread_join(self->thread);
  
  ecthread_delete(&(self->thread));
  
  ece_context_delete (&(self->econtext));
  
  free(self->matrix);
  
  //inotify_done( self->notifd );
  
  ENTC_DEL( ptr, struct EcEventFiles_s );
}

/*------------------------------------------------------------------------*/

void ece_files_register(EcEventFiles self, const EcString filename, events_callback_fct onChange, void* onChangePtr, events_callback_fct onDelete, void* onDeletePtr)
{
  int ident = inotify_add_watch(self->notifd, filename, IN_ALL_EVENTS);
  
  if( ident < 0 )
  {
    eclogger_logerrno(self->logger, LL_ERROR, "CORE", "{ece_files} Can't register event for inotify");
    
    return;
  }
  
  if( ident > self->size )
  {
    ece_files_resize( self, ident + self->size );
  }
  
  EcEventFilesData* data = &(self->matrix[ident]);
  
  data->onChange = onChange;
  data->onChangePtr = onChangePtr;
  data->onDelete = onDelete;
  data->onDeletePtr = onDeletePtr;
  data->filename = ecstr_copy (filename);
  
  eclogger_logformat(self->logger, LL_DEBUG, "CORE", "{ece_files} Registered event at inotify on %i", ident);
  eclogger_logformat(self->logger, LL_TRACE, "CORE", "{ece_files} at '%s'", filename);
}

/*------------------------------------------------------------------------*/

#endif