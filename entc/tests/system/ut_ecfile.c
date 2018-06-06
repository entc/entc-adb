#include "tests/ecenv.h"
#include "system/ecfile.h"

#include <stdio.h>

//=============================================================================

static int __STDCALL test_copydir (void* ptr, TestEnvContext ctx, EcErr err)
{
  int res = ecfs_cpdir ("test_dir", "copy_dir", err);
  
  
  return res;
}

//=============================================================================

int main(int argc, char* argv[])
{
  TestEnv te = testenv_create ();
  
  testenv_reg (te, "Test1", NULL, NULL, test_copydir);
  
  testenv_run (te);
  
  return testenv_destroy (&te);
}

//---------------------------------------------------------------------------
