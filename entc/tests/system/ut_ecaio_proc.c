#include "system/ecaio.h"
#include "tests/ecenv.h"
#include "system/ecaio_proc.h"
#include "system/ecproc.h"

#include "system/ecthread.h"
#include "system/ecevents.h"

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

static int __STDCALL test_ecaio_parent_thread (void* ptr)
{
  EcErr err = ecerr_create();
  
  printf ("worker thread wait\n");
  
  ecaio_wait (ptr, ENTC_INFINITE, err);
  
  printf ("worker thread done\n");
  
  ecerr_destroy (&err);
  
  return 0;
}

//---------------------------------------------------------------------------

static int __STDCALL test_ecaio_test1_onNotify (void* ptr, int action)
{
  printf ("notify\n");
  
  return 0;
}

//---------------------------------------------------------------------------

static void __STDCALL test_ecaio_test1_onDestroy (void* ptr)
{
  printf ("destroy\n");
  
}

//---------------------------------------------------------------------------

static int __STDCALL test_ecaio_parent (void* ptr, TestEnvContext tctx, EcErr err)
{
  int res;
  EcAio aio = ecaio_create ();

  ecaio_init (aio, err);

  EcProc proc = ecproc_create ();
  
  res = ecproc_start (proc, "ut_ecaio_proc", "test", err);
  if (testctx_err (tctx, err))
  {
    return 0;
  }
  
  {
    EcAioProc ctx = ecaio_proc_create (ecproc_handle(proc));
    
    ecaio_proc_setCallback (ctx, NULL, test_ecaio_test1_onNotify, test_ecaio_test1_onDestroy);
    
    ecaio_proc_assign (&ctx, aio, err);
  }

  ecaio_registerTerminateControls (aio, FALSE, err);
  
  {
    EcThread thread = ecthread_new();
    
    ecthread_start (thread, test_ecaio_parent_thread, aio);

    printf ("#1\n");
    
    ece_sleep (10000);

    printf ("#2\n");

    ecproc_terminate (proc);
    
    printf ("#3\n");

    ece_sleep (100);
    
    printf ("#4\n");

    res = ecproc_waitForProcessToTerminate (proc, err);
    if (testctx_err (tctx, err))
    {

    }
    
    printf ("#5\n");

    // this should not trigger
    kill(getpid(), SIGTERM);
    
    printf ("#6\n");

    ecthread_join(thread);
    
    ecthread_delete(&thread);
  }
  
  ecproc_destroy(&proc);
  
  ecaio_destroy (&aio);
  
  return 0;
}

//---------------------------------------------------------------------------

static int __STDCALL test_ecaio_client (void* ptr, TestEnvContext tctx, EcErr err)
{
  EcAio aio = ecaio_create ();
  
  ecaio_init (aio, err);
  
  ecaio_registerTerminateControls (aio, FALSE, err);

  int res = ecaio_wait(aio, ENTC_INFINITE, err);
  if (res)
  {
    printf("wait abort: %s\n", err->text);
  }
  
  ecaio_destroy (&aio);
  
  return 0;
}

//=============================================================================

int main(int argc, char* argv[])
{
  TestEnv te = testenv_create ();
  
  if (argc > 1)
  {
    printf ("CLIENT\n");

    testenv_reg (te, "Test Client", test_ecaio_init, test_ecaio_done, test_ecaio_client);
  }
  else
  {
    printf ("PARENT\n");

    testenv_reg (te, "Test Parent", test_ecaio_init, test_ecaio_done, test_ecaio_parent);
  }
  
  testenv_run (te);
  
  return testenv_destroy (&te);
}

//---------------------------------------------------------------------------
