#include "system/ecaio.h"
#include "tests/ecenv.h"

#include "system/ecthread.h"
#include "system/ecevents.h"

#include <stdio.h>

//=============================================================================

static void* __STDCALL test_ecaio_init ()
{
  return ecaio_create ();
}

//---------------------------------------------------------------------------

static void __STDCALL test_ecaio_done (void* ptr)
{
  EcAio aio = ptr;
  
  ecaio_destroy (&aio);
}

//---------------------------------------------------------------------------

static int __STDCALL test_ecaio_test1_thread (void* ptr)
{
  EcErr err = ecerr_create();
  
  printf ("worker thread wait\n");
  
  ecaio_wait (ptr, ENTC_INFINITE, err);

  printf ("worker thread done\n");

  ecerr_destroy (&err);
  
  return 0;
}

//---------------------------------------------------------------------------

static int __STDCALL test_ecaio_test1 (void* ptr, TestEnvContext ctx, EcErr err)
{
  EcAio aio = ptr;

  ecaio_init (aio, err);
  
  if (testctx_err (ctx, err))
  {
    
  }
  
  {
    EcThread thread = ecthread_new();
    
    ecthread_start (thread, test_ecaio_test1_thread, aio);
    
    ece_sleep (1000);
    
    ecaio_abort(aio, err);
    if (testctx_err (ctx, err))
    {
      
    }
    
    ecthread_join(thread);
    
    ecthread_delete(&thread);
  }
  
  return 0;
}

//---------------------------------------------------------------------------

static int __STDCALL test_ecaio_test2 (void* ptr, TestEnvContext ctx, EcErr err)
{
  EcAio aio = ptr;
  
  ecaio_init (aio, err);
  
  if (testctx_err (ctx, err))
  {
    
  }
  
  {
    int i;
    
    EcThread thread [10];
    
    for (i = 0; i < 10; i++)
    {
      thread [i] = ecthread_new();
      
      ecthread_start (thread [i], test_ecaio_test1_thread, aio);
    }
    
    ece_sleep (1000);
    
    ecaio_abort(aio, err);
    if (testctx_err (ctx, err))
    {
      
    }
    
    for (i = 0; i < 10; i++)
    {
      ecthread_join(thread [i]);
      
      ecthread_delete(&(thread [i]));
    }
  }
  
  return 0;
}

//=============================================================================

int main(int argc, char* argv[])
{
  TestEnv te = testenv_create ();
  
  testenv_reg (te, "Single Thread", test_ecaio_init, test_ecaio_done, test_ecaio_test1);
  testenv_reg (te, "Multi Thread", test_ecaio_init, test_ecaio_done, test_ecaio_test2);
  
  testenv_run (te);
  
  return testenv_destroy (&te);
}

//---------------------------------------------------------------------------
