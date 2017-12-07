
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
  EcBuffer textToEncode = ecbuf_create_fromStr ("1234567890abcdefghif");
  
  EcBuffer sha256 = ecbuf_sha_256 (textToEncode, err);
  

  testctx_push_string (tctx, ecbuf_const_str(sha256));
  
  testctx_assert (tctx, testctx_pop_tocomp (tctx, "df276bb3814778f574bd5ff0f8d889625726240d563b277997b87bba91e67596"), "sha256 compare");
  
  ecbuf_destroy (&sha256);
  ecbuf_destroy (&textToEncode);
  
  return 0;
}

//=============================================================================

int main(int argc, char* argv[])
{
  TestEnv te = testenv_create ();
  
  testenv_reg (te, "SHA256", test_ecbuffer_init, test_ecbuffer_done, test_ecbuffer_sha256);
  
  testenv_run (te);
  
  return testenv_destroy (&te);
}

//---------------------------------------------------------------------------
