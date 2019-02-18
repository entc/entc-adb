/*
 Copyright (c) 2019 Alexander Kalkhof
 
 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:
 
 The above copyright notice and this permission notice shall be included in all
 copies or substantial portions of the Software.
 
 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 SOFTWARE.
 */

#include "entc_queue.h"

// entc includes
#include "sys/entc_mutex.h"
#include "sys/entc_thread.h"
#include "stc/entc_list.h"

// c includes
#include <stdio.h>

//-----------------------------------------------------------------------------

struct EntcQueue_s
{
  EntcMutex mutex;

  EntcMutex waitm;  // TODO: use a semaphore instead ??
  
  EntcThread thread;
  
  EntcList queue;
  
  int locked;
};

//-----------------------------------------------------------------------------

struct EntcQueueItem_s
{
  entc_queue_cb_fct on_event;

  entc_queue_cb_fct on_done;

  void* ptr;
  
}; typedef struct EntcQueueItem_s* EntcQueueItem;

//-----------------------------------------------------------------------------

void __STDCALL entc_queue_item_on_del (void* ptr)
{
  EntcQueueItem item = ptr;
  
  if (item->on_done)
  {
    item->on_done (item->ptr);
  }

  ENTC_DEL(&item, struct EntcQueueItem_s);
}

//-----------------------------------------------------------------------------

EntcQueue entc_queue_new (void)
{
  EntcQueue self = ENTC_NEW(struct EntcQueue_s);

  self->mutex = entc_mutex_new ();
  self->waitm = entc_mutex_new ();

  self->thread = NULL;
  
  self->queue = entc_list_new (entc_queue_item_on_del);
  
  // block next
  entc_mutex_lock (self->waitm);
  self->locked = TRUE;

  return self;    
}

//-----------------------------------------------------------------------------

void entc_queue_del (EntcQueue* p_self)
{
  EntcQueue self = *p_self;
  
  entc_mutex_lock (self->mutex);

  if (self->locked)
  {
    entc_mutex_unlock (self->waitm);
    self->locked = FALSE;
  }
  
  // remove all queued items
  entc_list_clr (self->queue);
  
  entc_mutex_unlock (self->mutex);

  if (self->thread)
  {
    entc_thread_join (self->thread);
    
    entc_thread_del (&(self->thread));
  }

  entc_list_del (&(self->queue));
  
  entc_mutex_del (&(self->mutex));
  entc_mutex_del (&(self->waitm));

  ENTC_DEL(p_self, struct EntcQueue_s);
}

//-----------------------------------------------------------------------------

static int __STDCALL entc_queue_worker_thread (void* ptr)
{
  while (entc_queue_next (ptr));
  
  return 0;
}

//-----------------------------------------------------------------------------

int entc_queue_background  (EntcQueue self, int amount_of_threads, EntcErr err)
{
  self->thread = entc_thread_new ();
  
  entc_thread_start (self->thread, entc_queue_worker_thread, self);
  
  return ENTC_ERR_NONE;
}

//-----------------------------------------------------------------------------

void entc_queue_add (EntcQueue self, entc_queue_cb_fct on_event, entc_queue_cb_fct on_done, void* ptr)
{
  entc_mutex_lock (self->mutex);
  
  {
    EntcQueueItem item = ENTC_NEW (struct EntcQueueItem_s);
    
    item->on_done = on_done;
    item->on_event = on_event;
    item->ptr = ptr;
    
    entc_list_push_back (self->queue, item);

    // signal that we can process something
    entc_mutex_unlock (self->waitm);
    self->locked = FALSE;
  }
  
  entc_mutex_unlock (self->mutex);
}

//-----------------------------------------------------------------------------

int entc_queue_next (EntcQueue self)
{
  int ret = TRUE;
  EntcQueueItem item = NULL;
  
  // wait until a next element was pushed
  entc_mutex_lock (self->waitm);
  
  entc_mutex_lock (self->mutex);
  
  {
    number_t queue_size = entc_list_size (self->queue);
    
    if (queue_size)
    {
      item = entc_list_pop_front (self->queue);
    }
    else
    {
      ret = FALSE;
    }
  }
  
  {
    number_t queue_size = entc_list_size (self->queue);
    
    if (queue_size)
    {
      // signal that we can process more
      entc_mutex_unlock (self->waitm);
      self->locked = FALSE;
    }
    else
    {
      self->locked = TRUE;
    }
  }

  entc_mutex_unlock (self->mutex);
  
  if (item)
  {
    if (item->on_event)
    {
      item->on_event (item->ptr);
    }

    if (item->on_done)
    {
      item->on_done (item->ptr);
    }
    
    ENTC_DEL(&item, struct EntcQueueItem_s);
  }
  
  return ret;
}

//-----------------------------------------------------------------------------
