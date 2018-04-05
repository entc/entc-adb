#include "tests/ecenv.h"
#include "system/macros.h"

#include "types/ecbuffer.h"

#include "tools/eccrypt.h"

//---------------------------------------------------------------------------

static int __STDCALL test_crypt_test1 (void* ptr, TestEnvContext tctx, EcErr err)
{
  EcBuffer buf;
  EcDecryptAES dec;
  
  EcBuffer_s data;
  
  data.buffer = (const char*)"U2FsdGVkX19nJLLyxozdLWV5mBklYSDB65xlDysflRI=";
  data.size = strlen(data.buffer);
  
  EcBuffer data2 = ecbuf_decode_base64(&data);
  
  EcBuffer data3 = ecbuf_bin2hex(data2);
  
  printf ("%s\n", data3->buffer);
  
  dec = ecdecrypt_aes_initialize ("mysecretvault", ENTC_AES_TYPE_CFB, ENTC_KEY_PASSPHRASE, err);
  if (dec == NULL)
  {
    return err->code;
  }
  
  buf = ecdecrypt_aes_update (dec, data2, err);
  if (buf == NULL)
  {
    return err->code;    
  }

  buf = ecdecrypt_aes_finalize (dec, err);
  if (buf == NULL)
  {
    return err->code;    
  }
  
  printf ("%s\n", buf->buffer);
  
  ecdecrypt_aes_destroy (&dec);
  
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
