#include "tests/ecenv.h"

#include "types/ecbuffer.h"
#include "types/ecstream.h"
#include "types/ecudc.h"

#include "tools/eccode.h"
#include "tools/ecjson.h"

#include <stdio.h>

//---------------------------------------------------------------------------

static int __STDCALL test_crypt_test1 (void* ptr, TestEnvContext tctx, EcErr err)
{
  EcBuffer buf;
  EcBuffer_s data;
  
  data.buffer = (unsigned char*)"wir lieben die sonne";
  data.size = strlen((const char*)data.buffer);
  
  buf = echash_sha256 (&data, err);
  if (buf == NULL)
  {
    return err->code;
  }
  
  testctx_assert (tctx, buf->size == 32, "buffer size not correct");
  
  {
    EcBuffer h = ecbuf_bin2hex (buf);
  
    testctx_push_string (tctx, h->buffer);
    
    if (testctx_pop_tocomp (tctx, "58d42090cd2f263a087d1c2c591636a2a6e26f3a2f348eff69ce0c2df19ff8ea"))
    {
      
      
    }
    
    ecbuf_destroy (&h);
  }
  
  ecbuf_destroy (&buf);
  
  return ENTC_ERR_NONE;
}

//---------------------------------------------------------------------------

static int __STDCALL test_base64_test1 (void* ptr, TestEnvContext tctx, EcErr err)
{
  EcBuffer buf1;
  EcBuffer buf2;
  EcBuffer_s data;
  
  data.buffer = (unsigned char*)"wir lieben die sonne";
  data.size = strlen((const char*)data.buffer);
  
  buf1 = eccode_base64_encode (&data);

  if (!ecstr_equal (buf1->buffer, "d2lyIGxpZWJlbiBkaWUgc29ubmU="))
  {
    return ecerr_set (err, ENTC_LVL_ERROR, ENTC_ERR_WRONG_VALUE, "comparison failed #1");
  }

  printf ("%s\n", buf1->buffer);
  
  buf2 = eccode_base64_decode (buf1);
  
  printf ("%s\n", buf2->buffer);
  
  if (!ecstr_equal (buf2->buffer, "wir lieben die sonne"))
  {
    return ecerr_set (err, ENTC_LVL_ERROR, ENTC_ERR_WRONG_VALUE, "comparison failed #2");
  }
  
  testctx_assert (tctx, buf2->size == data.size, "buffer size not correct");

  printf ("%lu | %lu\n", buf2->size, data.size);

  {
    EcBuffer h = ecbuf_bin2hex (buf2);
  
    printf ("%s\n", h->buffer);
    
    ecbuf_destroy (&h);
  }
  
  ecbuf_destroy (&buf1);
  ecbuf_destroy (&buf2);

  return ENTC_ERR_NONE;
}

//---------------------------------------------------------------------------

static int __STDCALL test_base64_test2 (void* ptr, TestEnvContext tctx, EcErr err)
{
  EcBuffer buf1;
  EcBuffer buf1_base64;
  EcBuffer buf2;
  EcBuffer buf3;
  EcBuffer buf3_base64;
  EcUdc node2;
  EcUdc node3;

  EcUdc node1 = ecudc_create(EC_ALLOC, ENTC_UDC_NODE, NULL);
  
  ecudc_add_asNumber (EC_ALLOC, node1, "num1", 23);
  ecudc_add_asNumber (EC_ALLOC, node1, "num2", 42);
  ecudc_add_asString (EC_ALLOC, node1, "num2", "hello world!!");
  
  buf1 = ecjson_write (node1);
  
  printf ("BUF1: %s\n", buf1->buffer);
  
  buf1_base64 = eccode_base64_encode (buf1);
  
  printf ("BA64: %s\n", buf1_base64->buffer);

  buf2 = eccode_base64_decode (buf1_base64);
  
  printf ("BUF2: %s\n", buf2->buffer);

  node2 = ecjson_read_ecbuf (buf2, NULL);
  
  buf3_base64 = ecbuf_create_str_cp ("eyJoYSI6IjAwMDE1MjQ1OTgzMjQ5MTAiLCJpZCI6IjQxMzVhYTlkYzFiODQyYTY1M2RlYTg0NjkwM2RkYjk1YmZiOGM1YTEwYzUwNGE3ZmExNmUxMGJjMzFkMWZkZjAiLCJkYSI6ImJlMzQ0ZTg0NDc0Nzk0NTU3MGMyZDBhYjE4Nzk5MDkzNDhkOTJlYzQ5YzliMWRhNmY2ODhmOGZmODYxNDg5ZDUiLCJ3cGlkIjoiMyJ9");

  buf3 = eccode_base64_decode (buf3_base64);

  printf ("BUF3: %s\n", buf3->buffer);

  node3 = ecjson_read_ecbuf (buf3, NULL);
  
  if (node3 == NULL)
  {
    printf ("no node\n");
  }
  
  return ENTC_ERR_NONE;
}

//---------------------------------------------------------------------------

int main(int argc, char* argv[])
{
  TestEnv te = testenv_create ();
  int i;
  
  testenv_reg (te, "SHA256 Test1", NULL, NULL, test_crypt_test1);
  testenv_reg (te, "BASE64 Test1", NULL, NULL, test_base64_test1);
  testenv_reg (te, "JSON Test1", NULL, NULL, test_base64_test2);
    
  testenv_run (te);
    
  return testenv_destroy (&te);
}
