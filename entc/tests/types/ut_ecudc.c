
#include "types/ecudc.h"
#include "tests/ecenv.h"

#include "tools/ecjson.h"

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
  const char* t1 = "{\"userid\":5,\"wpid\":3,\"token\":\"5643-1243-8421-7632-8799E18FCC23\",\"name\":\"alex\",\"roles\":{\"master_all\":\"role\",\"files_upload\":\"role\"}}";

  EcUdc udc01; 
  EcUdc udc02;

  udc01 = ecjson_read_s(t1, "t1");

  // print
  {
    EcString h = ecjson_toString(udc01);
    
    printf("T1: %s\n", h);
    
    ecstr_delete(&h);
  }

  udc02 = ecudc_clone(EC_ALLOC, udc01);
  
  // print
  {
    EcString h = ecjson_toString(udc02);
    
    printf("C1: %s\n", h);
    
    ecstr_delete(&h);
  }
  
  ecudc_destroy(EC_ALLOC, &udc01);
  ecudc_destroy(EC_ALLOC, &udc02);
  
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
