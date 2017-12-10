
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

static int __STDCALL test_stdlist_unwrapl (void* ptr, TestEnvContext tctx, EcErr err)
{
  const EcString s1 = "Hello World";
  const EcString s2 = "Hello World <tim>";
  const EcString s3 = "Hello <tim> World";
  const EcString s4 = "<tim> Hello World";

  // test 1
  {
    EcString l1;
    EcString r1;
    
    EcString e1 = ecstr_unwrapl (s1, '<', '>', &l1, &r1);

    testctx_assert (tctx, ecstr_equal (e1, "Hello World"), "#1");
    testctx_assert (tctx, l1 == NULL, "#2");
    testctx_assert (tctx, r1 == NULL, "#3");
    
    ecstr_delete(&e1);
    ecstr_delete(&l1);
    ecstr_delete(&r1);
  }

  // test 2
  {
    EcString l1;
    EcString r1;
    
    EcString e1 = ecstr_unwrapl (s2, '<', '>', &l1, &r1);
    
    testctx_assert (tctx, ecstr_equal (e1, "tim"), "#1");
    testctx_assert (tctx, ecstr_equal (l1, "Hello World "), "#2");
    testctx_assert (tctx, r1 == NULL, "#3");
    
    ecstr_delete(&e1);
    ecstr_delete(&l1);
    ecstr_delete(&r1);
  }

  // test 3
  {
    EcString l1;
    EcString r1;
    
    EcString e1 = ecstr_unwrapl (s3, '<', '>', &l1, &r1);
    
    testctx_assert (tctx, ecstr_equal (e1, "tim"), "#1");
    testctx_assert (tctx, ecstr_equal (l1, "Hello "), "#2");
    testctx_assert (tctx, ecstr_equal (r1, " World"), "#3");
    
    ecstr_delete(&e1);
    ecstr_delete(&l1);
    ecstr_delete(&r1);
  }

  // test 4
  {
    EcString l1;
    EcString r1;
    
    EcString e1 = ecstr_unwrapl (s4, '<', '>', &l1, &r1);
    
    testctx_assert (tctx, ecstr_equal (e1, "tim"), "#1");
    testctx_assert (tctx, l1 == NULL, "#2");
    testctx_assert (tctx, ecstr_equal (r1, " Hello World"), "#3");
    
    ecstr_delete(&e1);
    ecstr_delete(&l1);
    ecstr_delete(&r1);
  }

  return 0;
}

//=============================================================================

int main(int argc, char* argv[])
{
  TestEnv te = testenv_create ();
  
  testenv_reg (te, "", test_stdlist_init, test_stdlist_done, test_stdlist_unwrapl);
  
  testenv_run (te);
  
  return testenv_destroy (&te);
}

//---------------------------------------------------------------------------
