#include "tests/ecenv.h"
#include "system/macros.h"
#include "system/ecfile.h"

#include "tools/ecmime.h"

//---------------------------------------------------------------------------

static int __STDCALL test_mime_multipart (void* ptr, TestEnvContext tctx, EcErr err)
{
  EcFileHandle fh = ecfh_open ("message.eml", O_TRUNC | O_CREAT | O_RDWR);

  EcMultipart mp = ecmultipart_create (NULL, "From: John Doe <example@example.com>\r\n");
  
  ecmultipart_addText (mp, "Hello World!", "text");
  ecmultipart_addFile (mp, ".", "test.pdf", 1);
  
  // use a very small buffer
  EcBuffer buf = ecbuf_create(10000);
  
  while (TRUE)
  {
    uint_t res = ecmultipart_next (mp, buf);
    if (res == 0)
    {
      break;
    }

    ecfh_writeConst(fh, (char*)buf->buffer, res);
  }
  
  ecmultipart_destroy (&mp);
  
  ecbuf_destroy(&buf);
  
  ecfh_close (&fh);  
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
