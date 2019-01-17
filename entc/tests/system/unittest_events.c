#include "system/ecevents.h"

#include "utils/ecmessages.h"
#include "sys/entc_thread.h"
#include <stdio.h>

//--------------------------------------------------------------------------

int _STDCALL callback1 (void* ptr)
{
  ece_sleep (100);
  
  ece_context_setAbort (ptr);
  
  return FALSE;
}

typedef struct
{
  
  EcEventQueue queue;
  
  EcHandle handle;
  
} HandleProps;

//--------------------------------------------------------------------------

int _STDCALL callback2 (void* ptr)
{
  HandleProps* props = ptr;
  void* ptr2;
  int res;

  ece_sleep (100);
    
  ece_list_set (props->queue, props->handle);
  
  ece_sleep(100);
  
  res = ece_list_wait (props->queue, ENTC_INFINITE, &ptr2);

  printf("waited type %i with pointer %p\n", res, ptr2);
  
  res = ece_list_del (props->queue, props->handle);

  printf("try to delete %i\n", res);

  ece_list_set (props->queue, props->handle);

  props->handle = ece_list_handle (props->queue, props);

  ece_sleep(100);

  ece_list_set (props->queue, props->handle);
    
  return FALSE;
}

//--------------------------------------------------------------------------

int _STDCALL callback3 (void* ptr)
{
  HandleProps* props = ptr;
  void* ptr2 = NULL;
  int res;

  ece_sleep (100);
  
  ece_list_set (props->queue, props->handle);
  
  ece_sleep (100);
  
  res = ece_list_wait (props->queue, ENTC_INFINITE, &ptr2);
  
  printf("waited3 type %i with pointer %p\n", res == ENTC_EVENT_ABORT, ptr2);
  
  return FALSE;
}

//--------------------------------------------------------------------------

int _STDCALL callback4 (void* ptr)
{
  HandleProps* props = ptr;
  void* ptr2 = NULL;
  int res;
  
  res = ece_list_wait (props->queue, ENTC_INFINITE, &ptr2);
  
  printf("waited4 type %i with pointer %p\n", res == ENTC_EVENT_ABORT, ptr2);
  
  return FALSE;
}

//--------------------------------------------------------------------------

void onDelete (void** pptr)
{
  printf("deleted %p\n", *pptr);
  
}

//--------------------------------------------------------------------------

int main (int argc, char *argv[])
{
  time_t t1;
  int res;
  EcEventContext econtext = ece_context_new ();
 
  ecmessages_initialize ();
  
  t1 = time(0);
    
  res = ece_context_waitforAbort (econtext, 200);
  
  printf("waited for %ld ms with type %i\n", (long)(time(0) - t1), res == ENTC_EVENT_TIMEOUT);
  
  // simple tests with one thread
  {
    EntcThread th1 = entc_thread_new ();  
    entc_thread_start (th1, callback1, econtext);
    
    t1 = time(0);
    
    res = ece_context_waitforAbort (econtext, ENTC_INFINITE);

    printf("waited for %ld ms with type %i\n", (long)(time(0) - t1), res == ENTC_EVENT_ABORT);
      
    entc_thread_join(th1);
    entc_thread_del(&th1);
  }
  
  ece_context_resetAbort (econtext);

  // event queues
  {
    void* ptr;
    HandleProps p1;  
    HandleProps p2;  
    HandleProps p3;  

    EntcThread th2 = entc_thread_new ();  
    EntcThread th3 = entc_thread_new ();  
    EntcThread th4 = entc_thread_new ();  

    EcEventQueue equeue = ece_list_create (econtext, onDelete);
    
    t1 = time(0);
    
    res = ece_list_wait (equeue, 200, &ptr);
    
    printf("waited for %ld ms with type %i with pointer %p\n", (long)(time(0) - t1), res == ENTC_EVENT_TIMEOUT, ptr);

    p1.queue = equeue;
    p1.handle = ece_list_handle (equeue, &p1);
    
    entc_thread_start (th2, callback2, &p1);

    res = ece_list_wait (equeue, ENTC_INFINITE, &ptr);

    printf("waited for %ld ms with type %i with pointer %p\n", (long)(time(0) - t1), res, ptr);
    
    ece_sleep (100);
    
    ece_list_set (p1.queue, p1.handle);

    p2.queue = equeue;
    p2.handle = ece_list_handle (equeue, &p2);
    
    p3.queue = equeue;
    p3.handle = ece_list_handle (equeue, &p3);

    entc_thread_start (th3, callback3, &p2);
    
    t1 = time(0);

    res = ece_list_wait (equeue, ENTC_INFINITE, &ptr);

    printf("waited for %ld ms with type %i with pointer %p\n", (long)(time(0) - t1), res, ptr);
    
    t1 = time(0);
    
    res = ece_list_wait (equeue, ENTC_INFINITE, &ptr);
    
    printf("waited for %ld ms with type %i with pointer %p\n", (long)(time(0) - t1), res, ptr);

    entc_thread_start (th4, callback4, &p3);

    ece_sleep(200);
    
    ece_context_setAbort (econtext);
    
    entc_thread_join(th2);
    entc_thread_del(&th2);
    
    entc_thread_join(th3);
    entc_thread_del(&th3);
    
    entc_thread_join(th4);
    entc_thread_del(&th4);

    ece_list_destroy(&equeue); 
    ece_context_delete (&econtext);
  }
  
  ecmessages_deinitialize ();
  
  return 0;
}
