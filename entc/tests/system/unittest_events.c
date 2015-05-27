#include "system/ecevents.h"

#include "system/ecthread.h"
#include <stdio.h>

//--------------------------------------------------------------------------

int _STDCALL callback1 (void* ptr)
{
  sleep (1);
  
  ece_context_triggerTermination (ptr);
  
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
  sleep (1);
  
  HandleProps* props = ptr;
  
  ece_list_set (props->queue, props->handle);
  
  sleep(1);
  
  void* ptr2;
  
  int res = ece_list_wait (props->queue, ENTC_INFINTE, &ptr2);

  printf("waited type %i with pointer %p\n", res, ptr2);
  
  res = ece_list_del (props->queue, props->handle);

  printf("try to delete %i\n", res);

  ece_list_set (props->queue, props->handle);

  props->handle = ece_list_handle (props->queue, props);

  sleep(1);

  ece_list_set (props->queue, props->handle);
  
  sleep (2);

  res = ece_list_wait (props->queue, ENTC_INFINTE, &ptr2);
  
  printf("waited type %i with pointer %p\n", res == ENTC_EVENT_ABORT, ptr2);
  
  return FALSE;
}

//--------------------------------------------------------------------------

int _STDCALL callback3 (void* ptr)
{
  sleep (1);
  
  HandleProps* props = ptr;

  ece_list_set (props->queue, props->handle);
  
  sleep(2);
  
  void* ptr2;
  
  int res = ece_list_wait (props->queue, ENTC_INFINTE, &ptr2);
  
  printf("waited type %i with pointer %p\n", res == ENTC_EVENT_ABORT, ptr2);
  
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
  EcEventContext econtext = ece_context_new ();
 
  time_t t1 = time(0);
  
  int res;
  
  res = ece_context_waitforTermination (econtext, 200);
  
  printf("waited for %u ms with type %i\n", (long)(time(0) - t1), res == ENTC_EVENT_TIMEOUT);
  
  EcThread th1 = ecthread_new ();  
  ecthread_start (th1, callback1, econtext);
  
  t1 = time(0);
  
  res = ece_context_waitforTermination (econtext, ENTC_INFINTE);

  printf("waited for %u ms with type %i\n", (long)(time(0) - t1), res == ENTC_EVENT_ABORT);
    
  ecthread_join(th1);
  ecthread_delete(&th1);

  EcEventQueue equeue = ece_list_create (econtext, onDelete);

  void* ptr;
  
  t1 = time(0);
  
  res = ece_list_wait (equeue, 200, &ptr);
  
  printf("waited for %u ms with type %i with pointer %p\n", (long)(time(0) - t1), res == ENTC_EVENT_TIMEOUT, ptr);

  HandleProps p1;  
  p1.queue = equeue;
  p1.handle = ece_list_handle (equeue, &p1);
  
  EcThread th2 = ecthread_new ();  
  ecthread_start (th2, callback2, &p1);

  res = ece_list_wait (equeue, ENTC_INFINTE, &ptr);

  printf("waited for %u ms with type %i with pointer %p\n", (long)(time(0) - t1), res, ptr);
  
  sleep(1);
  
  ece_list_set (p1.queue, p1.handle);

  HandleProps p2;  
  p2.queue = equeue;
  p2.handle = ece_list_handle (equeue, &p2);
  
  EcThread th3 = ecthread_new ();  
  ecthread_start (th3, callback3, &p2);
  
  t1 = time(0);

  res = ece_list_wait (equeue, ENTC_INFINTE, &ptr);

  printf("waited for %u ms with type %i with pointer %p\n", (long)(time(0) - t1), res, ptr);
  
  t1 = time(0);
  
  res = ece_list_wait (equeue, ENTC_INFINTE, &ptr);
  
  printf("waited for %u ms with type %i with pointer %p\n", (long)(time(0) - t1), res, ptr);

  sleep(2);
  
  ece_context_triggerTermination (econtext);
  
  ecthread_join(th2);
  ecthread_delete(&th2);
  
  ecthread_join(th3);
  ecthread_delete(&th3);
  
  ece_context_delete (&econtext);
  
  return 0;
}
