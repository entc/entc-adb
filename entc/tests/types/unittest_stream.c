// utils includes
#include "utils/ecmessages.h"

// types includes
#include "types/ecbuffer.h"
#include "types/ecstream.h"

//-------------------------------------------------------------------------------------------

int main (int argc, char *argv[])
{
  ecmessages_initialize ();
  
  EcBuffer buf1 = ecbuf_create (10);
  ecbuf_fill (buf1, 10, '*');

  EcBuffer buf2 = ecbuf_create (10);
  ecbuf_fill (buf2, 10, '#');

  EcStream s1 = ecstream_new ();
  
  printf("00 '%s'\n", ecstream_buffer (s1));
  
  ecstream_append (s1, "test");
         
  printf("01 '%s'\n", ecstream_buffer (s1));

  ecstream_append (s1, " test2");
  
  printf("02 '%s'\n", ecstream_buffer (s1));

  ecstream_appendd (s1, (char*)buf1->buffer, buf1->size);
  
  printf("03 '%s'\n", ecstream_buffer (s1));

  ecstream_appendd (s1, (char*)buf2->buffer, buf2->size);
  
  printf("04 '%s'\n", ecstream_buffer (s1));

  ecstream_delete(&s1);
  
  return 0;
}

//-------------------------------------------------------------------------------------------
