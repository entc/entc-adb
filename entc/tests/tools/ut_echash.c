#include "tests/ecenv.h"
#include "system/macros.h"

#include "types/ecbuffer.h"
#include "types/ecstream.h"

#include "tools/echash.h"

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
    
    
  }
  
  printf("E %lu :%s\n", buf->size, buf->buffer);

  {
    EcBuffer h = ecbuf_bin2hex (buf);
    
    printf("H %lu :%s\n", h->size, h->buffer);
  }
  
  
  return ENTC_ERR_NONE;
}

//---------------------------------------------------------------------------

int main(int argc, char* argv[])
{
  TestEnv te = testenv_create ();
  int i;
  
  testenv_reg (te, "Crypt Test1", NULL, NULL, test_crypt_test1);
    
  
 
  testenv_run (te);
    
  return testenv_destroy (&te);
}
