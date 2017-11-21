
#include "types/ecmap.h"
#include "tests/ecenv.h"

#include <stdio.h>

//=============================================================================

static void __STDCALL test_stdlist_onItemDestroy (void* key, void* val)
{
  
}

//---------------------------------------------------------------------------

static void* __STDCALL test_ecmap_init (EcErr err)
{
  return NULL;
}

//---------------------------------------------------------------------------

static void __STDCALL test_ecmap_done (void* ptr)
{

}

//---------------------------------------------------------------------------

static int __STDCALL test_ecmap_test1 (void* ptr, TestEnvContext ctx, EcErr err)
{
  EcMap map01 = ecmap_create (NULL, test_stdlist_onItemDestroy);
  
  
  ecmap_destroy(&map01);
  
  return 0;
}

//=============================================================================

int main(int argc, char* argv[])
{
  TestEnv te = testenv_create ();
  
  testenv_reg (te, "Test1", test_ecmap_init, test_ecmap_done, test_ecmap_test1);
  
  testenv_run (te);
  
  return testenv_destroy (&te);
}

//---------------------------------------------------------------------------
