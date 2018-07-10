#include "adbo2.h"

// entc includes
#include <tests/ecenv.h>

//=============================================================================

static int __STDCALL main_test (void* ptr, TestEnvContext ectx, EcErr err)
{
  EcString path = ecfs_getCurrentDirectory();
  
  Adbo2 adbo = adbo2_create (path, path, "adbo");

  Adbo2Context ctx = adbo2_ctx (adbo, NULL, NULL);
  if (ctx)
  {
    Adbo2Transaction trx = adbo2_ctx_transaction (ctx);
    
    // query
    {
      EcUdc data = ecudc_create (EC_ALLOC, ENTC_UDC_NODE, NULL);
      EcUdc para = ecudc_create (EC_ALLOC, ENTC_UDC_NODE, NULL);
      
      ecudc_add_asNumber (EC_ALLOC, para, "id", 1);
      
      int res = adbo2_trx_query (trx, "test_table01", para, data, err);
      if (res)
      {
        return res;
      }
      
      ecudc_destroy (EC_ALLOC, &para);
      ecudc_destroy (EC_ALLOC, &data);
    }
    
    adbo2_trx_commit (&trx);
    
    adbo2_ctx_destroy (&ctx);
  }
    
  adbo2_destroy (&adbo);

  ecstr_delete (&path);

  return ENTC_ERR_NONE;
}

//---------------------------------------------------------------------------

int main(int argc, char* argv[])
{
  TestEnv te = testenv_create ();
  
  testenv_reg (te, "Main", NULL, NULL, main_test);
  testenv_run (te);
  
  return testenv_destroy (&te);
}

//-----------------------------------------------------------------------------

