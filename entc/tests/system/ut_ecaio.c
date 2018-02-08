#include "aio/ecaio.h"
#include "tests/ecenv.h"

#include "system/ecthread.h"

#include <stdio.h>
#include <signal.h>

#ifdef _WIN32

#include <windows.h>

#else

#include <unistd.h>

#endif

//=============================================================================

static void* __STDCALL test_ecaio_init (EcErr err)
{
  return NULL;
}

//---------------------------------------------------------------------------

static void __STDCALL test_ecaio_done (void* ptr)
{
}

//---------------------------------------------------------------------------

static int __STDCALL test_ecaio_test1_thread (void* ptr)
{
  EcErr err = ecerr_create();
  
  printf ("worker thread wait\n");
  
  ecaio_wait (ptr, err);

  printf ("worker thread done\n");

  ecerr_destroy (&err);
  
  return 0;
}

//---------------------------------------------------------------------------

static int __STDCALL test_ecaio_test1 (void* ptr, TestEnvContext ctx, EcErr err)
{
  EcAio aio = ecaio_create ();

  ecaio_init (aio, err);
  
  if (testctx_err (ctx, err))
  {
    
  }
  
  {
    EcThread thread = ecthread_new(NULL);
    
    ecthread_start (thread, test_ecaio_test1_thread, aio);
    
    ecthread_sleep (1000);
    
    ecaio_abort(aio, err);
    if (testctx_err (ctx, err))
    {
      
    }
    
    ecthread_join(thread);
    
    ecthread_delete(&thread);
  }
  
  ecaio_destroy (&aio);

  return 0;
}

//---------------------------------------------------------------------------

static int __STDCALL test_ecaio_test2 (void* ptr, TestEnvContext ctx, EcErr err)
{
  EcAio aio = ecaio_create ();
  
  ecaio_init (aio, err);
  
  if (testctx_err (ctx, err))
  {
    
  }
  
  {
    int i;
    
    EcThread thread [10];
    
    for (i = 0; i < 10; i++)
    {
      thread [i] = ecthread_new(NULL);
      
      ecthread_start (thread [i], test_ecaio_test1_thread, aio);
    }
    
    ecthread_sleep (1000);
    
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
  
  ecaio_destroy (&aio);

  return 0;
}

//---------------------------------------------------------------------------

static int __STDCALL test_ecaio_test3 (void* ptr, TestEnvContext ctx, EcErr err)
{
  EcAio aio = ecaio_create ();
  
  ecaio_init (aio, err);
  
  ecaio_registerTerminateControls (aio, TRUE, err);

  if (testctx_err (ctx, err))
  {
    
  }
  
  {
    int i;
    
    EcThread thread [10];
    
    for (i = 0; i < 10; i++)
    {
      thread [i] = ecthread_new(NULL);
      
      ecthread_start (thread [i], test_ecaio_test1_thread, aio);
    }
    
    ecthread_sleep (1000);
    
#ifdef _WIN32

    printf ("send SIGINT\n");

	GenerateConsoleCtrlEvent (CTRL_C_EVENT, 0);

	ece_sleep (1000);

	printf ("send SIGTERM\n");

	GenerateConsoleCtrlEvent (CTRL_CLOSE_EVENT, 0);

#else

    printf ("send SIGINT\n");

    // this should not trigger
    kill(getpid(), SIGINT);
 
    ecthread_sleep (1000);

    printf ("send SIGTERM\n");
 
    // this will abort
    kill(getpid(), SIGTERM);

#endif

    for (i = 0; i < 10; i++)
    {
      ecthread_join(thread [i]);
      
      ecthread_delete(&(thread [i]));
    }
  }
  
  ecaio_destroy (&aio);

  return 0;
}

//---------------------------------------------------------------------------

static int __STDCALL test_ecaio_test4 (void* ptr, TestEnvContext ctx, EcErr err)
{
  EcAio aio = ecaio_create ();
  
  ecaio_init (aio, err);
  
  ecaio_registerTerminateControls (aio, FALSE, err);

  if (testctx_err (ctx, err))
  {
    
  }
  
  {
    int i;
    
    EcThread thread [10];
    
    for (i = 0; i < 10; i++)
    {
      thread [i] = ecthread_new(NULL);
      
      ecthread_start (thread [i], test_ecaio_test1_thread, aio);
    }
    
    ecthread_sleep (1000);
    
#ifdef _WIN32
    
    GenerateConsoleCtrlEvent (CTRL_C_EVENT, 0);
    
#else
    
    printf ("send SIGINT\n");
    
    // this should not trigger
    kill(getpid(), SIGINT);
    
#endif
    
    for (i = 0; i < 10; i++)
    {
      ecthread_join(thread [i]);
      
      ecthread_delete(&(thread [i]));
    }
  }
  
  ecaio_destroy (&aio);

  return 0;
}

//=============================================================================

int main(int argc, char* argv[])
{
  TestEnv te = testenv_create ();
  
  testenv_reg (te, "Single Thread", test_ecaio_init, test_ecaio_done, test_ecaio_test1);
  testenv_reg (te, "Multi Thread", test_ecaio_init, test_ecaio_done, test_ecaio_test2);
  testenv_reg (te, "Signal INT", test_ecaio_init, test_ecaio_done, test_ecaio_test4);
  testenv_reg (te, "Signal TERM", test_ecaio_init, test_ecaio_done, test_ecaio_test3);
  
  testenv_run (te);
  
  return testenv_destroy (&te);
}

//---------------------------------------------------------------------------
