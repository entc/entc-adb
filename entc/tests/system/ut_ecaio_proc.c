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
  return ecaio_create ();
}

//---------------------------------------------------------------------------

static void __STDCALL test_ecaio_done (void* ptr)
{
  EcAio aio = ptr;
  
  ecaio_destroy (&aio);
}

//---------------------------------------------------------------------------

static int __STDCALL test_ecaio_parent_thread (void* ptr)
{
  EcErr err = ecerr_create();
  
  printf ("worker thread wait\n");
  
  ecaio_wait_abortOnSignal (ptr, TRUE, err);
  
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
  EcAio aio = ptr;

  EcProc proc = ecproc_create ();

  ecaio_init (aio, err);
  
  res = ecproc_start (proc, "ut_ecaio_proc", "test", err);
  if (testctx_err (tctx, err))
  {
    return 0;
  }
  
  printf ("$0\n");

  {
    EcAioProc ctx = ecaio_proc_create (ecproc_handle(proc));
    
    ecaio_proc_setCallback (ctx, NULL, test_ecaio_test1_onNotify, test_ecaio_test1_onDestroy);
    
    ecaio_proc_assign (&ctx, aio, err);
  }
    
  printf ("$1\n");

  ecaio_reset_signals (err);

  printf ("$2\n");
  
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

#ifdef _WIN32

	GenerateConsoleCtrlEvent (CTRL_C_EVENT, 0);

#else

	// this should not trigger
    raise(SIGTERM);

#endif

    
    printf ("#6\n");

    ecthread_join(thread);
    
    ecthread_delete(&thread);
  }
  
  ecproc_destroy(&proc);
  
  return 0;
}

//---------------------------------------------------------------------------

static int __STDCALL test_ecaio_client (void* ptr, TestEnvContext tctx, EcErr err)
{
  int res;
  EcAio aio = ptr;
  
  ecaio_init (aio, err);
  
  res = ecaio_wait_abortOnSignal(aio, TRUE, err);
  if (res)
  {
    printf("wait abort: %s\n", err->text);
  }
  
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
