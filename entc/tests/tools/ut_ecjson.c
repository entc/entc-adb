
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
  \"path\" : \"/usr/local/lib/libadbl-1.3.0\",\
  \"databases\" :\
  [\
   {\
   \"name\" : \"default\",\
   \"type\" : \"Mysql\",\
   \"host\" : \"localhost\",\
   \"port\" : 3306,\
   \"user\" : \"safesign\",\
   \"pass\" : \"hadsh56!hsg\",\
   \"schema\" : \"lobo_esign_01\"\
   }\
   ]\
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

static int __STDCALL test_ecjson_test3 (void* ptr, TestEnvContext tctx, EcErr err)
{
  EcUdc data;
  
  const char* test1 = "{\"settings\":{\"host\":\"127.0.0.1\",\"port\":\"8084\"}}";
  
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
  
 // testenv_reg (te, "Json Reader Test1", test_ecjson_init, test_ecjson_done, test_ecjson_test1);
 // testenv_reg (te, "Json Reader Test2", test_ecjson_init, test_ecjson_done, test_ecjson_test2);
  testenv_reg (te, "Json Reader Test3", test_ecjson_init, test_ecjson_done, test_ecjson_test3);
  
  testenv_run (te);
  
  return testenv_destroy (&te);
}

//---------------------------------------------------------------------------
