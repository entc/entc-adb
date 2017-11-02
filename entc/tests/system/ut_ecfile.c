
#include "types/ecmap.h"
#include "tests/ecenv.h"

#include <stdio.h>

//=============================================================================

static void* __STDCALL test_stdlist_init ()
{
  return NULL;
}

//---------------------------------------------------------------------------

static void __STDCALL test_stdlist_done (void* ptr)
{

}

//---------------------------------------------------------------------------

static int __STDCALL test_stdlist_test1 (void* ptr, TestEnvContext ctx, EcErr err)
{
  
  return 0;
}

//=============================================================================

int main(int argc, char* argv[])
{
  TestEnv te = testenv_create ();
  
  testenv_reg (te, "Test1", test_stdlist_init, test_stdlist_done, test_stdlist_test1);
  
  testenv_run (te);
  
  return testenv_destroy (&te);
}

//---------------------------------------------------------------------------
