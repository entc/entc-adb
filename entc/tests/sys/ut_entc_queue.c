#include "sys/entc_queue.h"
#include "sys/entc_thread.h"

#include <stdio.h>

//-----------------------------------------------------------------------------

void __STDCALL test_cb_01 (void* ptr)
{
  printf ("CB 01\n");
}

//-----------------------------------------------------------------------------

void __STDCALL test_cb_02 (void* ptr)
{
  printf ("CB 02\n");
}

//-----------------------------------------------------------------------------

int main (int argc, char *argv[])
{
  EntcErr err = entc_err_new();
  
  {
    EntcQueue queue = entc_queue_new ();
    
    // start the background thread
    entc_queue_background (queue, 1, err);
    
    // add something
    entc_queue_add (queue, test_cb_01, test_cb_02, NULL);
    entc_queue_add (queue, test_cb_01, test_cb_02, NULL);
    entc_queue_add (queue, test_cb_01, test_cb_02, NULL);
    entc_queue_add (queue, test_cb_01, test_cb_02, NULL);

    // give some time to run in background
    entc_thread_sleep (1000);
    
    entc_queue_del (&queue);
  }

  {
    EntcQueue queue = entc_queue_new ();
    
    // start the background thread
    //entc_queue_background (queue, 1, err);

    
    entc_queue_del (&queue);
  }
  
  entc_err_del (&err);

  return 0;
}

//-----------------------------------------------------------------------------
