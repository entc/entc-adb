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

struct EcEventContext_s
{
  
  int pipe[2];
  
};

struct EcEventQueue_s
{
  
  EcEventContext ec;
  
  EcMutex mutex;
  
  int maxfd;
    
  int fds_r[20];
  int fdp_r;
  
  int fds_w[20];
  int fdp_w;

  int sfds_r[10];
  int sfds_w[10];
  int sfd;
  
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
}

//------------------------------------------------------------------------------------------------------------

void ece_context_delete (EcEventContext* pself)
{
  EcEventContext self = *pself;
  
  if( self->pipe[1] > 0 )
  {
    close(self->pipe[0]);
    close(self->pipe[1]);
  }
  
  ENTC_DEL(pself, struct EcEventContext_s);
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
    FD_SET(self->pipe[0], &fdset);      

    if (timeout == ENTC_INFINTE)
    {
      retval = select(self->pipe[0] + 1, &fdset, NULL, NULL, NULL);      
    }
    else
    {
      tv.tv_sec = timeout / 1000;
      tv.tv_usec = (timeout % 1000) * 1000;
      
      retval = select(self->pipe[0] + 1, &fdset, NULL, NULL, &tv);
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
      if( FD_ISSET(self->pipe[0], &fdset) )
      {
        reactOnAbort (self->pipe[0], self->pipe[1]);        
        rt = ENTC_EVENT_ABORT;
      }
      else
      {
        rt = ENTC_EVENT_UNKNOWN;
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
  if( self->pipe[1] > 0 )
  {
    write(self->pipe[1], "\n", 1);    
  }  
}

//------------------------------------------------------------------------------------------------------------

EcEventQueue ece_queue_new (EcEventContext ec)
{
  EcEventQueue self = ENTC_NEW(struct EcEventQueue_s);

  self->ec = ec;
  self->mutex = ecmutex_new ();
  
  self->maxfd = 0;

  // just for internal use
  self->fdp_r = 0;
  self->fdp_w = 0;
  
  self->sfd = 0;
  
  // add the read end of the pipe to react on aborts
  ece_queue_add (self, self->ec->pipe[0], ENTC_EVENTTYPE_USER);
  
  return self;  
}

//------------------------------------------------------------------------------------------------------------

void ece_queue_delete (EcEventQueue* sptr)
{
  EcEventQueue self = *sptr;
  
  int i;
  for (i = 0; i < self->sfd; i++)
  {
    // close all self assigned pipes
    close (self->sfds_r[i]);
    close (self->sfds_w[i]);
  }
  
  ecmutex_delete (&(self->mutex));
  
  ENTC_DEL (sptr, struct EcEventQueue_s);  
}

//------------------------------------------------------------------------------------------------------------

int ece_queue_size (EcEventQueue self)
{
}

//------------------------------------------------------------------------------------------------------------

void ece_queue_add_nts (EcEventQueue self, EcHandle handle, int type)
{
  switch (type)
  {
    case ENTC_EVENTTYPE_READ:
    {
      // add handle to our list
      self->fds_r[self->fdp_r] = handle;
      self->fdp_r++;
      // add handle to the fdset
      if (handle > self->maxfd) self->maxfd = handle;
    }
      break;
    case ENTC_EVENTTYPE_WRITE:
    {
      // add handle to our list
      self->fds_w[self->fdp_w] = handle;
      self->fdp_w++;
      // add handle to the fdset
      if (handle > self->maxfd) self->maxfd = handle;
    }
      break;
    case ENTC_EVENTTYPE_USER:
    {
      // add handle to our list
      self->fds_r[self->fdp_r] = handle;
      self->fdp_r++;
      if (handle > self->maxfd) self->maxfd = handle;
    }
      break;
  }  
}

//------------------------------------------------------------------------------------------------------------

void ece_queue_add (EcEventQueue self, EcHandle handle, int type)
{
  ecmutex_lock (self->mutex);
  
  ece_queue_add_nts (self, handle, type);
  
  ecmutex_unlock (self->mutex);
}

//------------------------------------------------------------------------------------------------------------

int ece_queue_wait_nts (EcEventQueue self, uint_t timeout, EcLogger logger, int* rt)
{
  struct timeval tv;
  int retval, i;
  //struct timeval t1, t2;
  // fd set must be create here, because select will change the content
  fd_set rfdset;
  fd_set wfdset;
  
  FD_ZERO(&rfdset);
  FD_ZERO(&wfdset);
  
  for (i = 0; i < self->fdp_r; i++)
  {
    FD_SET(self->fds_r[i], &rfdset);
  }
  for (i = 0; i < self->fdp_w; i++)
  {
    FD_SET(self->fds_w[i], &wfdset);
  }
  
  //gettimeofday(&t1, NULL);
  //eclogger_logformat(logger, LOGMSG_DEBUG, "CORE", "{queue::pipe} select on max:%i", self->maxfd);    
  if (timeout == ENTC_INFINTE)
  {
    retval = select(self->maxfd + 1, &rfdset, self->fdp_w > 0 ? &wfdset : NULL, NULL, NULL);      
  }
  else 
  {
    tv.tv_sec = timeout / 1000;
    tv.tv_usec = (timeout % 1000) * 1000;
    
    //eclogger_logformat(logger, LOGMSG_DEBUG, "CORE", "{queue::pipe} wait for timeout sec:%lu usec:%lu maxfd:%i", tv.tv_sec, tv.tv_usec, self->maxfd);            
    
    retval = select(self->maxfd + 1, &rfdset, self->fdp_w > 0 ? &wfdset : NULL, NULL, &tv);
  }
    
  // check error
  if (retval == -1)
  {
    if (errno == EINTR)
    {
      // try again
      return TRUE;      
    }
    else
    {
      eclogger_logerrno(logger, LL_WARN, "CORE", "{queue::pipe} error on select"); 
      
      *rt = ENTC_EVENT_ERROR;      
      return FALSE;
    }
  }  

  if (retval == 0)
  {
    eclogger_log(logger, LL_DEBUG, "CORE", "{queue::pipe} timeout on select");  
    *rt = ENTC_EVENT_TIMEOUT;
    return FALSE;
  }
  
  // abort
  if( FD_ISSET(self->ec->pipe[0], &rfdset) )
  {
    reactOnAbort (self->ec->pipe[0], self->ec->pipe[1]);
    eclogger_log(logger, LL_DEBUG, "CORE", "{queue::pipe} received abort event");      
    *rt = ENTC_EVENT_ABORT;
    
    return FALSE;
  } 
  
  // check read fds
  for (i = 0; i < self->fdp_r; i++)
  {
    if( FD_ISSET(self->fds_r[i], &rfdset) )
    {
      int j;
      for (j = 0; j < self->sfd; j++)
      {
        // check if its a user trigger
        if (self->sfds_r[j] == self->fds_r[i])
        {
          char buffer;
          int bytesread = read(self->sfds_r[j], &buffer, 1);
          if (bytesread < 0)
          {
            // some fatal error
            *rt = ENTC_EVENT_ERROR;
            return FALSE;            
          }
          else if (bytesread == 0)
          {
            // fd closed
            *rt = ENTC_EVENT_ERROR;
            return FALSE;
          }
          else
          {
            *rt = ENTC_EVENT_ABORT + i + 1;
            return FALSE;            
          }
        }
      }
            
      *rt = ENTC_EVENT_ABORT + i + 1;
      
      return FALSE;
    }
  }
  
  // check write fds
  for (i = 0; i < self->fdp_w; i++)
  {
    if( FD_ISSET(self->fds_w[i], &wfdset) )
    {
      *rt = ENTC_EVENT_ABORT + i + 1;
      
      return FALSE;
    }
  }
  
  eclogger_log(logger, LL_WARN, "CORE", "{queue::pipe} unknown event received");      
  *rt = ENTC_EVENT_UNKNOWN;
  
  return FALSE;
}

//------------------------------------------------------------------------------------------------------------

int ece_queue_wait (EcEventQueue self, uint_t timeout, EcLogger logger)
{
  int rt = -1;
  int nr = TRUE;
  
  while (nr)
  {
    ecmutex_lock (self->mutex);
                
    nr = ece_queue_wait_nts (self, timeout, logger, &rt);
    
    ecmutex_unlock (self->mutex);
    
  }
  return rt;  
}

//------------------------------------------------------------------------------------------------------------

EcHandle ece_queue_gen (EcEventQueue self)
{
  int lp[2];
  int ret;
  // 0 = Read-End; 1 = WriteEnd
  pipe(lp);
  // make the both ends noneblocking  
  ret = fcntl(lp[0], F_SETFL, fcntl( lp[0], F_GETFL, 0 ) | O_NONBLOCK );  
  if (ret < 0) {
    return -1;
  }
  ret = fcntl(lp[1], F_SETFL, fcntl( lp[1], F_GETFL, 0 ) | O_NONBLOCK );
  if (ret < 0) {
    return -1;
  }
  // keep the read end
  
  ecmutex_lock (self->mutex);
  
  self->sfds_r[self->sfd] = lp[0];
  self->sfds_w[self->sfd] = lp[1];
  self->sfd++;
  // add
  ece_queue_add_nts (self, lp[0], ENTC_EVENTTYPE_USER);

  ecmutex_unlock (self->mutex);
  // return the write end
  return lp[1];
}

//------------------------------------------------------------------------------------------------------------

void ece_queue_set (EcEventQueue self, EcHandle handle)
{
  // just write something into the write end
  if (handle > 0)
  {
    write(handle, "\n", 1);    
  }
}

//------------------------------------------------------------------------------------------------------------

void ece_queue_del (EcEventQueue self, EcHandle* phandle)
{
  EcHandle handle = *phandle;
  close (handle);
  *phandle = 0;
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
  
  EcEventQueue queue = ece_queue_new (self->econtext);
  // add the inotify file descriptor
  ece_queue_add (queue, self->notifd, ENTC_EVENTTYPE_READ);
  
  while (TRUE) 
  {
    int len;
    uint_t i = 0;
    char buf[EVENT_BUF_LEN];

    // wait until some data received on one of the handles
    eclogger_log(self->logger, LL_TRACE, "CORE", "{ece_files} wait for events");

    int res = ece_queue_wait (queue, ENTC_INFINTE, self->logger);
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
  
  ece_queue_delete (&queue);
  
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