#include "tests/ecenv.h"
#include "tools/eclog.h"
#include "tools/ecjson.h"

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
  EcString text;
  
  const char* test1 = "{\
  \"Name\":\"Hello World\",\
  \"Text\":\"\\\"SEQ\\\"TAB\tTAB\\t\",\
  \"Number\":42,\
  \"Double\":23.42,\
  \"True\":true,\
  \"False\":false,\
  \"Null\":null,\
  \"List\":[\"Text\",42,23.42,true,false,null],\
  \"Node\":{\"Text\":\"Hello\"}}";
  
  data = ecjson_read_s (test1, NULL);
  if (!data)
  {
    return 1;
  }
  
  text = ecjson_toString (data);
  
  eclog_fmt (LL_INFO, "TEST", "data", text);
  
  ecstr_delete(&text);
  ecudc_destroy(EC_ALLOC, &data);

  return 0;
}

//---------------------------------------------------------------------------

static int __STDCALL test_ecjson_test2 (void* ptr, TestEnvContext tctx, EcErr err)
{
  EcUdc data;
  EcString text;
  
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

  data = ecjson_read_s (test1, NULL);
  if (!data)
  {
    return 1;
  }
  
  text = ecjson_toString (data);
  
  eclog_fmt (LL_INFO, "TEST", "data", text);
  
  ecstr_delete(&text);
  ecudc_destroy(EC_ALLOC, &data);
  
  return 0;
}

//---------------------------------------------------------------------------

static int __STDCALL test_ecjson_test3 (void* ptr, TestEnvContext tctx, EcErr err)
{
  EcUdc data;
  EcString text;
  
  const char* test1 = "{\"settings\":{\"host\":\"127.0.0.1\",\"port\":\"8084\"}}";
  
  data = ecjson_read_s (test1, NULL);
  if (!data)
  {
    return 1;
  }
  
  text = ecjson_toString (data);
  
  eclog_fmt (LL_INFO, "TEST", "data", text);
  
  ecstr_delete(&text);
  ecudc_destroy(EC_ALLOC, &data);
  
  return 0;
}

//---------------------------------------------------------------------------

static int __STDCALL test_ecjson_test4 (void* ptr, TestEnvContext tctx, EcErr err)
{
  EcUdc data;
  
  const char* tests [7] =
  
  {
     "{}"
    ,"[]"
    ,"{\"_ctype\":1,\"_rinfo\":{\"userid\":5,\"wpid\":3,\"token\":\"5643-1243-8421-7632-8799E18FCC23\",\"name\":\"alex\",\"roles\":{\"master_all\":\"role\",\"files_upload\":\"role\"}},\"_cpath\":[\"view_processes_allval\"]}"
    ,"{\"_ctype\":1,\"_rinfo\":{\"userid\":5,\"wpid\":3,\"token\":\"5643-1243-8421-7632-8799E18FCC23\",\"name\":\"alex\",\"roles\":{\"master_all\":\"role\",\"files_upload\":\"role\"}},\"_cpath\":[\"view_processes_allval\"]}"
    ,"{\"userid\":\"undefined\",\"x\":1,\"y\":0}"
    ,"{\"userid\":\"undefined\",\"x\":-1,\"y\":-2}"
    ,"{\"_cdata\":{\"type\":\"Crypt4\",\"content\":\"eyJoYSI6IjAwMDE1MTEyOTU4ODkzNDkiLCJpZCI6IjQxMzVhYTlkYzFiODQyYTY1M2RlYTg0NjkwM2RkYjk1YmZiOGM1YTEwYzUwNGE3ZmExNmUxMGJjMzFkMWZkZjAiLCJkYSI6IjdkYWQwZjc0ZGU2NzVkNjZiODIxOWNhMzM1NjIzOWIyOWFlZmYzN2I0Y2IwMmY1NjU3MDJhMzZjNjk1M2U1MDIifQ==\"},\"_ctype\":0}"
  };
  
  int i;
  for (i = 0; i < 7; i++)
  {
    EcString text;

    data = ecjson_read_s (tests[i], NULL);
    if (!data)
    {
      return 1;
    }
    
    text = ecjson_toString (data);
    
    eclog_fmt (LL_INFO, "TEST", "data", text);
    
    ecstr_delete(&text);
    ecudc_destroy(EC_ALLOC, &data);
  }
  
  return 0;
}

//---------------------------------------------------------------------------

static int __STDCALL test_ecjson_test5 (void* ptr, TestEnvContext tctx, EcErr err)
{
  EcUdc data;
  
  data = ecjson_read_s (NULL, NULL);
  if (data)
  {
    return 1;
  }
  
  
  
  return 0;
}

//---------------------------------------------------------------------------

static int __STDCALL test_ecjson_test6 (void* ptr, TestEnvContext tctx, EcErr err)
{
  int i;
  EcUdc data;
  
  data = ecudc_create(EC_ALLOC, ENTC_UDC_LIST, "root");
  
  for (i = 0; i < 10; i++)
  {
    EcUdc item = ecudc_create(EC_ALLOC, ENTC_UDC_NODE, NULL);
    
    ecudc_add_asNumber(EC_ALLOC, item, "index", i);
    
    ecudc_add (data, &item);
  }
  
  ecjson_writeToFile (".meta2", data, "michi79", 0);
  
  ecudc_destroy(EC_ALLOC, &data);
  
  return 0;
}

//---------------------------------------------------------------------------

static int __STDCALL test_ecjson_test7 (void* ptr, TestEnvContext tctx, EcErr err)
{
  EcUdc data;
  EcString key;
  EcString h;

  key = ecstr_copy ("michi79");
  
  ecjson_readFromFile(".meta2", &data, key, 0);  // use the default encryption

  printf ("read done\n");

  h = ecjson_toString (data);
  
  printf ("%s\n", h);
  
  ecstr_delete(&h);
  
  ecudc_destroy(EC_ALLOC, &data);

  ecstr_delete(&key);

  return 0;
}

//---------------------------------------------------------------------------

static int __STDCALL test_ecjson_test8 (void* ptr, TestEnvContext tctx, EcErr err)
{
  int i;
  EcUdc data;
  
  data = ecudc_create(EC_ALLOC, ENTC_UDC_LIST, "root");
  
  for (i = 0; i < 10; i++)
  {
    EcUdc item = ecudc_create(EC_ALLOC, ENTC_UDC_NODE, NULL);
    
    ecudc_add_asNumber(EC_ALLOC, item, "index", i);
    
    ecudc_add (data, &item);
  }
  
  ecjson_writeToFile (".meta3", data, NULL, 0);
  
  ecudc_destroy(EC_ALLOC, &data);
  
  return 0;
}

//---------------------------------------------------------------------------

static int __STDCALL test_ecjson_test9 (void* ptr, TestEnvContext tctx, EcErr err)
{
  EcUdc data;
  EcString h;

  ecjson_readFromFile (".meta3", &data, NULL, 0);
  
  printf ("read done\n");
  
  h = ecjson_toString (data);
  
  printf ("%s\n", h);
  
  ecstr_delete(&h);
  
  ecudc_destroy(EC_ALLOC, &data);
  
  return 0;
}

//---------------------------------------------------------------------------

static int __STDCALL test_ecjson_test10 (void* ptr, TestEnvContext tctx, EcErr err)
{
  EcUdc data;
  
  int res = ecjson_readFromFile ("json02.txt", &data, NULL, 0);
  if (res)
  {
    return res;
  }
  
  printf ("read done\n");
  
  if (data)
  {
    EcString h = ecjson_toString (data);
    
    printf ("data: %s\n", h);
    
    ecstr_delete(&h);
  }
  else
  {
    printf ("no data\n");
  }
  
  ecudc_destroy(EC_ALLOC, &data);
  
  return 0;
}

//---------------------------------------------------------------------------

static int __STDCALL test_ecjson_test11 (void* ptr, TestEnvContext tctx, EcErr err)
{
  EcString h;
  const EcString h2;
  EcUdc node;
  EcUdc node2;
  const EcString unicodedText = "'\n\r\t\b\f/äאט流水";
  
  node = ecudc_create (EC_ALLOC, ENTC_UDC_NODE, NULL);
  
  ecudc_add_asString (EC_ALLOC, node, "text1", unicodedText);
  
  h = ecjson_toString (node);
  
  printf ("%s\n", h);
  
  node2 = ecjson_read_s(h, NULL);

  h2 = ecudc_get_asString (node2, "text1", NULL);
  
  if (!ecstr_equal(h2, unicodedText))
  {
    printf ("ERROR: not euqal\n");
  }
  
  return ENTC_ERR_NONE;
}

//---------------------------------------------------------------------------

int main(int argc, char* argv[])
{
  TestEnv te = testenv_create ();
  int i;
  
  testenv_reg (te, "Json Reader Test1", test_ecjson_init, test_ecjson_done, test_ecjson_test1);
  
  testenv_reg (te, "Json Reader Test2", test_ecjson_init, test_ecjson_done, test_ecjson_test2);
  testenv_reg (te, "Json Reader Test3", test_ecjson_init, test_ecjson_done, test_ecjson_test3);
  testenv_reg (te, "Json Reader Test4", test_ecjson_init, test_ecjson_done, test_ecjson_test4);
  testenv_reg (te, "Json Reader Test5", test_ecjson_init, test_ecjson_done, test_ecjson_test5);
  
  testenv_reg (te, "Json Reader Test6", test_ecjson_init, test_ecjson_done, test_ecjson_test6);
  testenv_reg (te, "Json Reader Test7", test_ecjson_init, test_ecjson_done, test_ecjson_test7);

  testenv_reg (te, "Json Reader Test8", test_ecjson_init, test_ecjson_done, test_ecjson_test8);
  testenv_reg (te, "Json Reader Test9", test_ecjson_init, test_ecjson_done, test_ecjson_test9);

  //testenv_reg (te, "Json Reader Test10", test_ecjson_init, test_ecjson_done, test_ecjson_test10);
   
  testenv_reg (te, "Json Reader Test11", NULL, NULL, test_ecjson_test11);

  //for (i = 0; i < 1000; i++)
//{
  //printf ("[%i]\n", i);
  
  testenv_run (te);
//}
  
  //U2FsdGVkX19nJLLyxozdLWV5mBklYSDB65xlDysflRI=
  
  return testenv_destroy (&te);
}

//---------------------------------------------------------------------------
