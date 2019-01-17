#include "tests/ecenv.h"
#include "system/ecfile.h"

#include "tools/ecmime.h"
#include "tools/entc_multipart.h"
#include "tools/eccrypt.h"

//---------------------------------------------------------------------------

static int __STDCALL test_mime_multipart (void* ptr, TestEnvContext tctx, EcErr err)
{
  int res = 0;
  
  EcBuffer buf;
  EcFileHandle fh = ecfh_open ("message.eml", O_TRUNC | O_CREAT | O_RDWR);

  
  const EcString secret = "khasdgasd78384geyag8723gb";
  
  {
    int res = ecencrypt_file ("test.pdf", "test.enc", secret, 0, err);
    if (res)
    {
      goto exit; 
    }
    
  }
  
  EntcMultipart mp = entc_multipart_new (NULL, "From: John Doe <example@example.com>\r\n");
  
  //EcMultipart mp = ecmultipart_create (NULL, "From: John Doe <example@example.com>\r\n");
  
  entc_multipart_add_text (mp, "Hello World!", "text");
  
  //  ecmultipart_addFile (mp, ".", "test.pdf", 1);
  entc_multipart_add_path (mp, "./test.enc", "test.pdf", 1, secret, 0);

  entc_multipart_add_path (mp, "./test.pdf", "test_original.pdf", 1, NULL, 0);
  
  // use a very small buffer
  buf = ecbuf_create(300);
  
  while (TRUE)
  {
    uint_t res = entc_multipart_next (mp, buf);

//    uint_t res = ecmultipart_next (mp, buf);
    if (res == 0)
    {
      break;
    }

    ecfh_writeConst(fh, (char*)buf->buffer, res);
  }
  
  entc_multipart_del (&mp);
  
  //ecmultipart_destroy (&mp);
  
  ecbuf_destroy(&buf);
  
  goto exit;
  
exit:
  
  ecfh_close (&fh);
  
  return res;
}

//-------------------------------------------------------------------------------------------

int main (int argc, char *argv[])
{
  TestEnv te = testenv_create ();
  int i;
  
  testenv_reg (te, "MIME Multipart", NULL, NULL, test_mime_multipart);
    
  testenv_run (te);
    
  return testenv_destroy (&te);
}

//-------------------------------------------------------------------------------------------
