
#include "tools/ecjson.h"
#include "tests/ecenv.h"

#include "utils/eclogger.h"

//=============================================================================

static void* __STDCALL test_ecjson_init (EcErr err)
{
  return NULL;
}

//---------------------------------------------------------------------------

static void __STDCALL test_ecjson_done (void* ptr)
{
}

//---------------------------------------------------------------------------

static int __STDCALL test_ecjson_test1 (void* ptr, TestEnvContext tctx, EcErr err)
{
  EcUdc data;
  
  const char* test1 = "{\
  \"Name\":\"Hello World\",\
  \"Number\":42,\
  \"Double\":23.42,\
  \"True\":true,\
  \"False\":false,\
  \"Null\":null,\
  \"List\":[\"Text\",42,23.42,true,false,null],\
  \"Node\":{\"Text\":\"Hello\"}}";
  
  data = ecjson_read (test1, NULL);
  if (!data)
  {
    return 1;
  }
  
  EcString text = ecjson_write(data);
  
  eclogger_fmt (LL_INFO, "TEST", "data", text);
  
  ecstr_delete(&text);
  ecudc_destroy(EC_ALLOC, &data);

  return 0;
}

//---------------------------------------------------------------------------

static int __STDCALL test_ecjson_test2 (void* ptr, TestEnvContext tctx, EcErr err)
{
  EcUdc data;
  
  const char* test1 = "{\
  \"path\": \"files\",\
  \"filename\": \"6AC881E5-C271-4F7F-B2F4-1BABCDC378BF\",\
  \"content\": [\
  [{\
  \"left\": 2.89532293986637,\
  \"top\": 7.559055118110236,\
  \"width\": 67.26057906458797,\
  \"height\": 17.79527559055118,\
  \"party\": {}\
  }]\
  ],\
  \"parties\": {\
  \"5a8ac34e-8bae-4bda-8095-06aaf2bd50bc\": {\
  \"key\": \"5a8ac34e-8bae-4bda-8095-06aaf2bd50bc\",\
  \"userid\": 1,\
  \"name\": \"alex\"\
  }\
  }\
  }";
  
  data = ecjson_read (test1, NULL);
  if (!data)
  {
    return 1;
  }
  
  EcString text = ecjson_write(data);
  
  eclogger_fmt (LL_INFO, "TEST", "data", text);
  
  ecstr_delete(&text);
  ecudc_destroy(EC_ALLOC, &data);
  
  return 0;
}

//---------------------------------------------------------------------------

int main(int argc, char* argv[])
{
  TestEnv te = testenv_create ();
  
  testenv_reg (te, "Json Reader Test1", test_ecjson_init, test_ecjson_done, test_ecjson_test1);
 // testenv_reg (te, "Json Reader Test2", test_ecjson_init, test_ecjson_done, test_ecjson_test2);
  
  testenv_run (te);
  
  return testenv_destroy (&te);
}

//---------------------------------------------------------------------------
