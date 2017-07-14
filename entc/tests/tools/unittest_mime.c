#include "tools/ecmime.h"

// utils includes
#include "utils/ecmessages.h"
#include "utils/ecreadbuffer.h"

// types includes
#include "types/ecbuffer.h"

//-------------------------------------------------------------------------------------------

int main (int argc, char *argv[])
{
  ecmessages_initialize ();

  EcFileHandle fh = ecfh_open ("message.eml", O_TRUNC | O_CREAT | O_RDWR);

  EcMultipart mp = ecmultipart_create (NULL, "From: John Doe <example@example.com>\r\n");
  
  ecmultipart_addText (mp, "Hello World!", "text");
  ecmultipart_addFile (mp, "samples", "test.pdf", 1);
  
  // use a very small buffer
  EcBuffer buf = ecbuf_create(10);
  
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
  
  ecfh_close (&fh);
  
  ecmessages_deinitialize();
  
  return 0;
}

//-------------------------------------------------------------------------------------------
