#include "tests/ecenv.h"
#include "system/macros.h"

#include "types/ecbuffer.h"
#include "types/ecstream.h"

#include "tools/eccrypt.h"
#include "tools/eccode.h"

//---------------------------------------------------------------------------

static int __STDCALL test_crypt_test1 (void* ptr, TestEnvContext tctx, EcErr err)
{
  EcBuffer buf;
  EcDecryptAES dec;
  
  EcStream stream;
  
  EcBuffer_s data;
  
  
  data.buffer = (unsigned char*)"U2FsdGVkX19nJLLyxozdLWV5mBklYSDB65xlDysflRI=";
  data.size = strlen((const char*)data.buffer);
  
  stream = ecstream_create();
  
  {
    EcBuffer data2 = eccode_base64_decode (&data);
  
    dec = ecdecrypt_aes_create ("mysecretvault", ENTC_AES_TYPE_CFB, ENTC_KEY_PASSPHRASE_MD5);
  
    buf = ecdecrypt_aes_update (dec, data2, err);
    if (buf == NULL)
    {
      return err->code;    
    }

    ecbuf_destroy(&data2);
    
    printf("D %lu :%s\n", buf->size, buf->buffer);
    
    {
      EcBuffer h = ecbuf_bin2hex(buf);
      
      printf("F %lu :%s\n", h->size, h->buffer);
      
      ecbuf_destroy(&h);
    }
    
    ecstream_append_ecbuf(stream, buf);
  }
    
  buf = ecdecrypt_aes_finalize (dec, err);
  if (buf == NULL)
  {
    return err->code;    
  }
    
  printf("E %lu :%s\n", buf->size, buf->buffer);
    
  ecstream_append_ecbuf(stream, buf);
  
  printf ("%s\n", ecstream_get(stream));
  
  ecdecrypt_aes_destroy (&dec);
  
  ecstream_destroy(&stream);
  
  return ENTC_ERR_NONE;
}

//---------------------------------------------------------------------------

static int __STDCALL test_crypt_test2 (void* ptr, TestEnvContext tctx, EcErr err)
{
  EcBuffer data1 = ecbuf_create_str_cp ("Hello World!");
  EcStream s = ecstream_create();
  EcBuffer data2;
  
  {
    EcBuffer buf;
    
    EcEncryptAES enc = ecencrypt_aes_create ("qwe!#jHKk34", ENTC_AES_TYPE_CFB, ENTC_KEY_PASSPHRASE_MD5);
  
    buf = ecencrypt_aes_update (enc, data1, err);
    if (buf == NULL)
    {
      return err->code;
    }
  
    ecstream_append_ecbuf (s, buf);
  
    buf = ecencrypt_aes_finalize (enc, err);
    if (buf == NULL)
    {
      return err->code;
    }
    
    ecstream_append_ecbuf (s, buf);

    ecencrypt_aes_destroy (&enc);
  }

  data2 = ecstream_tobuf (&s);
  s = ecstream_create();
    
  {
    EcBuffer buf;
    
    EcDecryptAES dec = ecdecrypt_aes_create ("qwe!#jHKk34", ENTC_AES_TYPE_CFB, ENTC_KEY_PASSPHRASE_MD5);
    
    buf = ecdecrypt_aes_update (dec, data2, err);
    if (buf == NULL)
    {
      return err->code;
    }    
    
    ecstream_append_ecbuf (s, buf);
  
    buf = ecdecrypt_aes_finalize (dec, err);
    if (buf == NULL)
    {
      return err->code;
    }
    
    ecstream_append_ecbuf (s, buf);

    ecdecrypt_aes_destroy (&dec);    
  }

  {
    EcString h = ecstream_tostr(&s);
  
    printf ("RES: '%s'\n", h);

    ecstr_delete(&h);
  }
  
  ecbuf_destroy(&data1);
  ecbuf_destroy(&data2);
    
  return ENTC_ERR_NONE;
}

//---------------------------------------------------------------------------

int main(int argc, char* argv[])
{
  TestEnv te = testenv_create ();
  int i;
  
  testenv_reg (te, "Crypt Test1", NULL, NULL, test_crypt_test1);
  testenv_reg (te, "Crypt Test1", NULL, NULL, test_crypt_test2);
    
  testenv_run (te);
    
  return testenv_destroy (&te);
}
