
#include "tools/eclparser.h"
#include "tests/ecenv.h"

//=============================================================================

static void __STDCALL onLine (void* ptr, const char* line)
{

}

//---------------------------------------------------------------------------

static void* __STDCALL test_stdlparser_init (EcErr err)
{
  return NULL;
}

//---------------------------------------------------------------------------

static void __STDCALL test_stdlparser_done (void* ptr)
{

}

//---------------------------------------------------------------------------

static int __STDCALL test_stdlparser_test1 (void* ptr, TestEnvContext tctx, EcErr err)
{
  
  return 0;
}

//---------------------------------------------------------------------------

static int __STDCALL test_stdlparser_test2 (void* ptr, TestEnvContext tctx, EcErr err)
{
  
  return 0;
}

//---------------------------------------------------------------------------

int main(int argc, char* argv[])
{
  TestEnv te = testenv_create ();
  
  testenv_reg (te, "LineParser Test1", test_stdlparser_init, test_stdlparser_done, test_stdlparser_test1);
  testenv_reg (te, "LineParser Test2", test_stdlparser_init, test_stdlparser_done, test_stdlparser_test2);
  
  testenv_run (te);
  
  return testenv_destroy (&te);
}

//---------------------------------------------------------------------------
