
#include "types/ecbuffer.h"
#include "tests/ecenv.h"

#include <stdio.h>

//=============================================================================

static void* __STDCALL test_ecbuffer_init (EcErr err)
{
  return NULL;
}

//---------------------------------------------------------------------------

static void __STDCALL test_ecbuffer_done (void* ptr)
{

}

//---------------------------------------------------------------------------

static int __STDCALL test_ecbuffer_sha256 (void* ptr, TestEnvContext tctx, EcErr err)
{

  
  return 0;
}

//=============================================================================

int main(int argc, char* argv[])
{
  TestEnv te = testenv_create ();
  
  //testenv_reg (te, "SHA256", test_ecbuffer_init, test_ecbuffer_done, test_ecbuffer_sha256);
  
  testenv_run (te);
  
  return testenv_destroy (&te);
}

//---------------------------------------------------------------------------
