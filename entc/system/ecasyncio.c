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

#include "ecasyncio.h"

#include "system/ecmutex.h"
#include "system/ecthread.h"

#include "types/eclist.h"
#include "system/ecrefcnt.h"
#include "system/ectime.h"

//-----------------------------------------------------------------------------------------------------------

struct EcAsyncContext_s
{
  
  void* ptr;
  
  const EcAsyncContextCallbacks* callbacks;
  
  EcStopWatch stopwatch;
  
};

//-----------------------------------------------------------------------------------------------------------

EcAsyncContext ecasync_context_create (ulong_t timeout, const EcAsyncContextCallbacks* callbacks, void* ptr)
{
  EcAsyncContext self = ENTC_NEW (struct EcAsyncContext_s);
  
  self->callbacks = callbacks;
  self->ptr = ptr;
  
  self->stopwatch = ecstopwatch_create (timeout);  
  ecstopwatch_start (self->stopwatch);
  
  return self;
}

//-----------------------------------------------------------------------------------------------------------

int _STDCALL ecasync_context_hasTimedOut (void* obj, void* ptr)
{
  EcAsyncContext self = obj;
  EcStopWatch refWatch = ptr;

  return ecstopwatch_timedOutRef(self->stopwatch, refWatch);
}

//-----------------------------------------------------------------------------------------------------------

void ecasync_context_destroy (EcAsyncContext* pself)
{
  EcAsyncContext self = *pself;
  
  if (isAssigned (self->callbacks->destroy))
  {
    self->callbacks->destroy (&(self->ptr));
  }
  
  ecstopwatch_destroy (&(self->stopwatch));
  
  ENTC_DEL (pself, EcAsyncContext);
}

//-----------------------------------------------------------------------------------------------------------

struct EcAsync_s
{
  
  EcList threads;
  
};

//-----------------------------------------------------------------------------------------------------------

typedef struct 
{
  
  EcThread thread;
  
  EcEventContext ec;
  
  EcEventQueue queue;
  
  EcStopWatch stopwatch;
  
} EcAsyncThread;

//-----------------------------------------------------------------------------------------------------------

void ecasync_thread_del_item (void** pptr)
{
  ecasync_context_destroy ((EcAsyncContext*)pptr);
}

//-----------------------------------------------------------------------------------------------------------

EcAsyncThread* ecasync_thread_create ()
{
  EcAsyncThread* self = ENTC_NEW (EcAsyncThread);
  
  self->thread = ecthread_new ();
  self->ec = ece_context_new ();
  
  // create a new queue
  self->queue = ece_list_create (self->ec, ecasync_thread_del_item);
  
  self->stopwatch = ecstopwatch_create (0);
  
  return self;
}

//-----------------------------------------------------------------------------------------------------------

void ecasync_thread_destroy (EcAsyncThread** pself)
{
  EcAsyncThread* self = *pself;
  
  ecthread_join (self->thread);

  ecthread_delete (&(self->thread));
  ece_context_delete (&(self->ec));
  
  ecstopwatch_destroy (&(self->stopwatch));
  
  ENTC_DEL (pself, EcAsyncThread);
}

//-----------------------------------------------------------------------------------------------------------

static int _STDCALL ecasync_thread_run (void* ptr)
{
  EcAsyncThread* self = ptr;
  
  EcAsyncContext context;
  
  int res = ece_list_wait (self->queue, 1000, (void**)&context);  
  
  // update internal time of the stopwatch
  ecstopwatch_start (self->stopwatch);
  
  if ((res == ENTC_EVENT_TIMEOUT)) // timeout or interupt
  {
    ece_list_sortout (self->queue, ecasync_context_hasTimedOut, self->stopwatch);
    return TRUE;
  }
  
  if (res < 0) // termination of the process
  {
    return FALSE;  // tell the thread to terminate
  }
  
  if (isNotAssigned (context))
  {
    return FALSE;
  }
  
  if (isAssigned (context->callbacks->run))
  {
    if (context->callbacks->run (context->ptr))
    {
      ecstopwatch_start (context->stopwatch);
      return TRUE;
    }
  }
    
  //eclogger_msg (LL_TRACE, "ENTC", "async", "connection removed");
  
  ece_list_del (self->queue, context->callbacks->handle (context->ptr));
    
  return TRUE;
}

//-----------------------------------------------------------------------------------------------------------

void ecasync_thread_start (EcAsyncThread* self)
{
  ecthread_start (self->thread, ecasync_thread_run, self);
}

//-----------------------------------------------------------------------------------------------------------

void ecasync_thread_stop (EcAsyncThread* self)
{
  ece_context_setAbort (self->ec);
}

//-----------------------------------------------------------------------------------------------------------

void ecasync_thread_add (EcAsyncThread* self, EcAsyncContext* pcontext)
{
  EcAsyncContext context = *pcontext;
  
  if (isAssigned (context->callbacks->handle))
  {
    EcHandle handle = context->callbacks->handle (context->ptr);
    
    ece_list_add (self->queue, handle, ENTC_EVENTTYPE_READ, *pcontext);
    *pcontext = NULL;    
  }
  else
  {
    ecasync_context_destroy (pcontext);
  }
}

//-----------------------------------------------------------------------------------------------------------

void ecasync_create_threads (EcAsync self, int threads)
{
  int i;  
  for (i = 0; i < threads; i++)
  {
    EcAsyncThread* thread = ecasync_thread_create ();
    
    eclist_append(self->threads, thread);
    
    ecasync_thread_start (thread);
  }
}

//-----------------------------------------------------------------------------------------------------------

EcAsync ecasync_create (int threads)
{
  EcAsync self = ENTC_NEW (struct EcAsync_s);
  
  self->threads = eclist_new ();

  ecasync_create_threads (self, threads);
  
  return self;
}

//-----------------------------------------------------------------------------------------------------------

void ecasync_clear (EcAsync self)
{
  EcListCursor cursor;
  
  eclist_cursor (self->threads, &cursor);
  
  while (eclist_cnext (&cursor))
  {
    ecasync_thread_stop (cursor.value);
  }
  
  eclist_cursor (self->threads, &cursor);

  while (eclist_cnext (&cursor))
  {
    ecasync_thread_destroy ((EcAsyncThread**)&(cursor.value));
  }
  
  eclist_clear (self->threads);
}

//-----------------------------------------------------------------------------------------------------------

void ecasync_destroy (EcAsync* pself)
{
  EcAsync self = *pself;
  
  ecasync_clear (self);
  eclist_delete (&(self->threads));
  
  ENTC_DEL (pself, struct EcAsync_s);
}

//-----------------------------------------------------------------------------------------------------------

void ecasync_addSingle (EcAsync self, EcAsyncContext* pcontext)
{
  EcListCursor cursor;
  int min;
  EcAsyncThread* cthread;
  EcAsyncThread* mthread;
  
  // add context to the thread queue which has least entries
  
  eclist_cursor (self->threads, &cursor);

  if (!eclist_cnext (&cursor))
  {
    ecasync_context_destroy (pcontext);
    return;
  }
  
  cthread = cursor.value;
  min = ece_list_size (cthread->queue);
  mthread = cthread;
  
  while (eclist_cnext (&cursor))
  {
    int h;
    
    cthread = cursor.value;
    h = ece_list_size (cthread->queue);
    
    if (h < min)
    {
      min = h;
      mthread = cthread;
    }
  }
  
  ecasync_thread_add (mthread, pcontext);
}

//-----------------------------------------------------------------------------------------------------------

typedef struct { EcRefCnt ref; } EcAsyncSmartContext;

//-----------------------------------------------------------------------------------------------------------

static void _STDCALL ecasync_smartcontext_destroy (void** ptr)
{
  EcAsyncSmartContext* self = *ptr;

  if (ecrefcnt_dec (self->ref) == 0)
  {
    EcAsyncContext context = ecrefcnt_get (self->ref);
    
    ecasync_context_destroy (&context);
    
    ecrefcnt_delete(&(self->ref));
  }
  
  ENTC_DEL (ptr, EcAsyncSmartContext);
}

//-----------------------------------------------------------------------------------------------------------

static EcHandle _STDCALL ecasync_smartcontext_handle (void* ptr)
{
  EcAsyncSmartContext* self = ptr;
  
  EcAsyncContext context = ecrefcnt_get (self->ref);
  
  return context->callbacks->handle (context->ptr);
}

//-----------------------------------------------------------------------------------------------------------

static int _STDCALL ecasync_smartcontext_run (void* ptr)
{
  EcAsyncSmartContext* self = ptr;
  
  EcAsyncContext context = ecrefcnt_get (self->ref);
  
  return context->callbacks->run (context->ptr);  
}

//-----------------------------------------------------------------------------------------------------------

EcAsyncContext ecasync_smartcontext_create (EcAsyncContext* pcontext)
{
  EcAsyncSmartContext* self;
  EcAsyncContext derive;

  static const EcAsyncContextCallbacks callbacks = {ecasync_smartcontext_destroy, ecasync_smartcontext_handle, ecasync_smartcontext_run};

  self = ENTC_NEW (EcAsyncSmartContext);

  self->ref = ecrefcnt_new (*pcontext);
    
  derive = ecasync_context_create (ecstopwatch_timeout((*pcontext)->stopwatch), &callbacks, self);
  
  *pcontext = NULL;
  
  return derive;
}

//-----------------------------------------------------------------------------------------------------------

EcAsyncContext ecasync_smartcontext_clone (EcAsyncContext rhs)
{
  EcAsyncSmartContext* self = ENTC_NEW (EcAsyncSmartContext);
  EcAsyncSmartContext* srhs = rhs->ptr;
  
  self->ref = srhs->ref;    
  
  return ecasync_context_create (ecstopwatch_timeout(rhs->stopwatch), rhs->callbacks, self);
}

//-----------------------------------------------------------------------------------------------------------

void ecasync_addToAll (EcAsync self, EcAsyncContext* pcontext)
{
  EcListCursor cursor; eclist_cursor(self->threads, &cursor);

  if (eclist_cnext (&cursor))
  {
    EcAsyncContext context = ecasync_smartcontext_create (pcontext);
    
    EcAsyncThread* thread = cursor.value;
    
    while (eclist_cnext (&cursor))
    {
      EcAsyncThread* thread2 = cursor.value;

      EcAsyncContext context2 = ecasync_smartcontext_clone (context);
      
      ecasync_thread_add (thread2, &context2);
    }
    
    ecasync_thread_add (thread, &context);
  }  
}

//-----------------------------------------------------------------------------------------------------------
