
#include "tools/ectemplate.h"
#include "tests/ecenv.h"

#include <stdio.h>

//=============================================================================

static void* __STDCALL test_ectemplate_init (EcErr err)
{
  return ectemplate_create ();
}

//---------------------------------------------------------------------------

static void __STDCALL test_ectemplate_done (void* ptr)
{
  EcTemplate h = ptr;
  ectemplate_destroy (&h);
}

//---------------------------------------------------------------------------

static int __STDCALL test_ectemplate_test1_onText (void* ptr, const char* text)
{
  printf ("%s", text);
  
  return ENTC_ERR_NONE;
}

//---------------------------------------------------------------------------

static int __STDCALL test_ectemplate_test1_onFile (void* ptr, const char* file)
{
  
  return ENTC_ERR_NONE;
}

//---------------------------------------------------------------------------

static int __STDCALL test_ectemplate_test1 (void* ptr, TestEnvContext ctx, EcErr err)
{
  EcTemplate h = ptr;
  
  EcUdc data = ecudc_create (EC_ALLOC, ENTC_UDC_NODE, NULL);
  
  ecudc_add_asString (EC_ALLOC, data, "topic", "Hello");
  
  {
    EcUdc rowes = ecudc_create (EC_ALLOC, ENTC_UDC_LIST, "rows");
    
    // row 1
    {
      EcUdc row = ecudc_create (EC_ALLOC, ENTC_UDC_NODE, NULL);
      
      ecudc_add_asString (EC_ALLOC, row, "col1", "row1");
      ecudc_add_asNumber (EC_ALLOC, row, "type", 1 );
      
      ecudc_add_asNumber (EC_ALLOC, row, "value1", 42);      
      ecudc_add_asDouble (EC_ALLOC, row, "value2", -0.23);   
      
      ecudc_add_asNumber (EC_ALLOC, row, "lvl1", 1);      
      ecudc_add_asNumber (EC_ALLOC, row, "lvl2", 1);      

      {
        EcUdc content = ecudc_create (EC_ALLOC, ENTC_UDC_NODE, "content");

        ecudc_add_asString (EC_ALLOC, content, "text", "rainbow in the sky");
        
        ecudc_add (row, &content);
      }
      
      ecudc_add_asString (EC_ALLOC, row, "date", "2018-04-23 12:54:23");

      ecudc_add (rowes, &row);
    }
    
    // row 1
    {
      EcUdc row = ecudc_create (EC_ALLOC, ENTC_UDC_NODE, NULL);
      
      ecudc_add_asString (EC_ALLOC, row, "col1", "row2");
      ecudc_add_asNumber (EC_ALLOC, row, "type", 2);
      
      ecudc_add_asNumber (EC_ALLOC, row, "value1", -12);      
      ecudc_add_asDouble (EC_ALLOC, row, "value2", 23.45);      

      ecudc_add (rowes, &row);
    }
    
    ecudc_add (data, &rowes);
  }
  

  int res = ectemplate_compile_file (h, "templates", "test1", "en", err);
  
  ectemplate_apply (h, data, NULL, test_ectemplate_test1_onText, test_ectemplate_test1_onFile, err);
  testctx_err (ctx, err);
  
  ecudc_destroy(EC_ALLOC, &data);

  return 0;
}

//=============================================================================

int main(int argc, char* argv[])
{
  TestEnv te = testenv_create ();
  
  testenv_reg (te, "Template Test1", test_ectemplate_init, test_ectemplate_done, test_ectemplate_test1);
  
  testenv_run (te);
  
  return testenv_destroy (&te);
}

//---------------------------------------------------------------------------
