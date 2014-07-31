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

#include "ecevents.h"

#include "ecfile.h"
#include "ecthread.h"

#include <windows.h>

struct EcEventContext_s {

  HANDLE abort;

};

#define MAX_EVENTS 50

//---------------------------------------------------------------------------------------------------------------------

EcEventContext ece_context_new (void)
{
  EcEventContext self = ENTC_NEW(struct EcEventContext_s);

  self->abort = CreateEvent(NULL, TRUE, FALSE, NULL);

  return self;
}

//---------------------------------------------------------------------------------------------------------------------

void ece_context_delete (EcEventContext* pself)
{
  EcEventContext self = *pself;

  CloseHandle(self->abort);

  ENTC_DEL(pself, struct EcEventContext_s);
}

//---------------------------------------------------------------------------------------------------------------------

int ece_context_wait (EcEventContext self, EcHandle handle, uint_t timeout, int type)
{
  HANDLE hs[2] = {self->abort, handle};

  DWORD res = WaitForMultipleObjects (2, hs, FALSE, (timeout == ENTC_INFINTE) ? INFINITE : timeout);

  if(res == WAIT_ABANDONED_0)
  {
	  return ENTC_EVENT_ERROR;
  }
  if(res == WAIT_FAILED)
  {
    return ENTC_EVENT_ERROR;
  }
  if(res == WAIT_TIMEOUT)
  {
    return ENTC_EVENT_TIMEOUT;
  }
  if (res - WAIT_OBJECT_0 == 0)
  {
    return ENTC_EVENT_ABORT;
  }

  return 0;
}

//---------------------------------------------------------------------------------------------------------------------

int ece_context_waitforTermination (EcEventContext self, uint_t timeout)
{
  DWORD res = WaitForSingleObject(self->abort, (timeout == ENTC_INFINTE) ? INFINITE : timeout);

  return (res == WAIT_TIMEOUT) ? ENTC_EVENT_TIMEOUT : ENTC_EVENT_ABORT;
}

//---------------------------------------------------------------------------------------------------------------------

void ece_context_triggerTermination (EcEventContext self)
{
  SetEvent (self->abort);
}

//---------------------------------------------------------------------------------------------------------------------

struct EcEventQueue_s {

  HANDLE hs [MAX_EVENTS]; // maximum amount of possible handles that can windows manage

  void* ps [MAX_EVENTS];

  uint_t hsn;

  EcMutex mutex;

};

//---------------------------------------------------------------------------------------------------------------------

EcEventQueue ece_list_create (EcEventContext ec)
{
  EcEventQueue self = ENTC_NEW (struct EcEventQueue_s);

  self->hs [0] = ec->abort;
  self->hs [1] = CreateEvent (NULL, FALSE, FALSE, NULL);

  self->hsn = 2;

  self->mutex = ecmutex_new ();

  return self;
}

//---------------------------------------------------------------------------------------------------------------------

void ece_list_destroy (EcEventQueue* pself)
{
  EcEventQueue self = *pself;

  CloseHandle (self->hs [1]);

  ecmutex_delete (&(self->mutex));

  ENTC_DEL(pself, struct EcEventQueue_s);
}

//---------------------------------------------------------------------------------------------------------------------

int ece_list_add (EcEventQueue self, EcHandle handle, int type, void* ptr)
{
  ecmutex_lock (self->mutex);

  if (self->hsn < MAX_EVENTS)
  {
    self->ps [self->hsn] = ptr;
    self->hs [self->hsn] = handle;
    self->hsn++;

    ecmutex_unlock (self->mutex);
    // trigger rebuild
    SetEvent (self->hs [1]);
    return TRUE;
  }
  else
  {
    ecmutex_unlock (self->mutex);
    return FALSE;    
  }
}

//---------------------------------------------------------------------------------------------------------------------

int ece_list_del (EcEventQueue self, EcHandle handle)
{
  uint_t i;

  ecmutex_lock (self->mutex);

  for (i = 2; i < self->hsn; i++)
  {
    if (self->hs [i] == handle)
    {
      self->hsn--;
      self->hs [i] = self->hs [self->hsn];
      self->ps [i] = self->ps [self->hsn];

      ecmutex_unlock (self->mutex);
      return TRUE;      
    }
  }
  ecmutex_unlock (self->mutex);
  // nothing found
  return FALSE;
}

//---------------------------------------------------------------------------------------------------------------------

int ece_list_wait (EcEventQueue self, uint_t timeout, void** ptr, EcLogger logger)
{
  while (TRUE)
  {
    DWORD ind;
    DWORD res = WaitForMultipleObjects (self->hsn, self->hs, FALSE, (timeout == ENTC_INFINTE) ? INFINITE : timeout);
    if(res == WAIT_ABANDONED_0)
    {
      return ENTC_EVENT_ERROR;
    }
    if(res == WAIT_FAILED)
    {
      return ENTC_EVENT_ERROR;
    }
    if(res == WAIT_TIMEOUT)
    {
      return ENTC_EVENT_TIMEOUT;
    }

    ind = res - WAIT_OBJECT_0;

    if (ind == 0)
    {
      return ENTC_EVENT_ABORT;
    }
    if (ind == 1)
    {
      continue; 
    }

    if (isAssigned (ptr))
    {
      *ptr = self->ps [ind];
    }

    return ind;
  }
}

//---------------------------------------------------------------------------------------------------------------------

EcHandle ece_list_handle (EcEventQueue self, void* ptr)
{
  HANDLE h = CreateEvent (NULL, FALSE, FALSE, NULL);
  ece_list_add (self, h, ENTC_EVENTTYPE_USER, ptr);
  return h;
}

//---------------------------------------------------------------------------------------------------------------------

void ece_list_set (EcEventQueue self, EcHandle handle)
{
  SetEvent (handle);
}

//---------------------------------------------------------------------------------------------------------------------

typedef struct
{
  
  events_callback_fct onChange;
  
  void* onChangePtr;
  
  events_callback_fct onDelete;
  
  void* onDeletePtr;

  HANDLE handle;
  
} EcEventFilesData;

struct EcEventFiles_s
{
  
  EcThread thread;
    
  EcEventQueue equeue;
  
  EcList handles;
  
  /* reference */
  EcLogger logger;
  
};

/*------------------------------------------------------------------------*/

int ece_files_nextEventWait (EcEventFiles self, uint_t size, void* eventList, EcEventFilesData** events)
{ 
  EcEventFilesData* event;
  int ident;

  printf("events : try to wait\n");

  ident = ece_list_wait (self->equeue, INFINITE, NULL, self->logger);

  if( ident > 1 )
  {
    printf("ident notified %i\n", ident);
  }
  else
  {
    return 0;
  }

  {
	  /*
    FILE_NOTIFY_INFORMATION infos[32 * 1024];
    DWORD ret;

    int res = ReadDirectoryChangesW(handlelst[l_dwWait], infos, sizeof(infos), FALSE, FILE_NOTIFY_CHANGE_LAST_WRITE, &ret, NULL, NULL);

    if( res == 0 )
    {
      eclogger_logerrno(self->logger, LOGMSG_WARNING, "CORE", "{events} Can't read changes");
      
      return TRUE;
    }

	eclogger_logformat(self->logger, LOGMSG_DEBUG, "CORE", "{events} Event with '%i' data", ret);

	if( infos[0].Action == FILE_ACTION_MODIFIED )
	{
		
	}
	*/

  }

  

  event = events[ident];

  if( event->onChange )
  {
	  event->onChange( event->onChangePtr );
  }

  //FindNextChangeNotification( handlelst[l_dwWait] );

  return TRUE;
}

/*------------------------------------------------------------------------*/

int ece_files_nextEvent (EcEventFiles self)
{
  EcHandle* handlelst;
  EcEventFilesData** events;
  
  EcListNode node;
  /* create the handle list */
  
  uint_t size = eclist_size( self->handles );
  uint_t pos = 0;

  int res = FALSE;
  /* allocate memory */
  handlelst = (EcHandle*)malloc( sizeof(EcHandle) * size );
  events = (EcEventFilesData**)malloc( sizeof(EcEventFilesData*) * size );
  
  /* fill */
  for(node = eclist_first(self->handles); node != eclist_end(self->handles); node = eclist_next(node))
  {
    EcEventFilesData* eventdata = eclist_data(node);
    
    events[pos] = eventdata;
    handlelst[pos] = eventdata->handle;

	  printf("register handle %i on %i\n", handlelst[pos], pos + 2);

    ece_list_add (self->equeue, handlelst[pos], ENTC_EVENTTYPE_READ, NULL);

    pos++;
  }
  
  res = ece_files_nextEventWait (self, size, handlelst, events);


  free(handlelst);
  free(events);
  
  return res;
}

/*------------------------------------------------------------------------*/

int ecevents_thread (void* a)
{
  return ece_files_nextEvent(a);
}

/*------------------------------------------------------------------------*/
 
EcEventFiles ece_files_new (EcLogger logger)
{
  EcEventFiles self = ENTC_NEW (struct EcEventFiles_s);
  
  self->logger = logger;
  
  
  //self->handles = eclist_new();
  
  //QTHREAD_CREATE( self->tid, ecevents_thread, (void*)self, 0 );

  return self;
}

/*------------------------------------------------------------------------*/
 
void ece_files_delete (EcEventFiles* ptr)
{
  EcEventFiles self = *ptr;


  /* wait until the thread returns */
  //QTHREAD_JOIN( self->tid );


  ENTC_DEL (ptr, struct EcEventFiles_s);
}

/*------------------------------------------------------------------------*/

void ece_files_register (EcEventFiles self, const EcString filename, events_callback_fct onChange, void* onChangePtr, events_callback_fct onDelete, void* onDeletePtr)
{
  EcString path = ecfs_getDirectory( filename );
  
  //HANDLE pHandle;
  HANDLE notifyHandle;

    return;

  if( onChange == NULL )
  {
    /* no event to register */
    return;
  }

  notifyHandle = FindFirstChangeNotification(path, FALSE, FILE_NOTIFY_CHANGE_LAST_WRITE);
    
  if( notifyHandle ) 
  {
    EcEventFilesData* event = ENTC_NEW (EcEventFilesData);
  
    eclogger_logformat(self->logger, LL_TRACE, "CORE", "{events} Registered event on '%d'", notifyHandle);

    event->handle = notifyHandle;
    event->onChange = onChange;
    event->onChangePtr = onChangePtr;
    event->onDelete = onDelete;
    event->onDeletePtr = onDeletePtr;
    
    //FIXME add mutex
    eclist_append(self->handles, event);
  }
  else
  {
    eclogger_logerrno(self->logger, LL_ERROR, "CORE", "{events} Can't registered event");
  }
  
  ecstr_delete( &path );
}

/*------------------------------------------------------------------------*/

#endif
