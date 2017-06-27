#include "tools/ecmime.h"

// utils includes
#include "utils/ecmessages.h"
#include "utils/ecreadbuffer.h"

// types includes
#include "types/ecbuffer.h"

typedef struct
{
  
  EcFileHandle fh;
  
  
} TestData;

//-------------------------------------------------------------------------------------------

TestData* init (const EcString path)
{
  TestData* td = ENTC_NEW (TestData);
  
  EcString filename = ecfs_mergeToPath (path, "data.dat");
  
  td->fh = ecfh_open (filename, O_RDONLY);

  if (isNotAssigned (td->fh))
  {
    return NULL;
  }
  
  return td;
}

//-------------------------------------------------------------------------------------------

char* content_callback (void* ptr, char* buffer, ulong_t inSize, int* outRes)
{
  TestData* td = ptr;
  
  EcBuffer_s buf = {(unsigned char*)buffer, inSize};
  
  *outRes = ecfh_readBuffer (td->fh, &buf);

  return buffer;
}

//-------------------------------------------------------------------------------------------

int main (int argc, char *argv[])
{
  ecmessages_initialize ();
  
  {
    EcString path = ecfs_getCurrentDirectory ();
    
    TestData* td = init (path);
    
    if (isAssigned (td))
    {
      EcHttpContent hc = echttp_content_create2 (NULL, NULL);
      
      EcMultipartParser mp = ecmultipartparser_create ("----WebKitFormBoundaryagZuqZ2AyfbmPxMl", path, content_callback, td, hc, NULL, NULL);
      
      int res = ecmultipartparser_process (mp, 0);
      if (res == ENTC_RESCODE_OK)
      {
        
      }
      
      EcHttpContent nc = echttp_content_next (hc);
      
      while (nc)
      {
        EcBuffer buf = echttp_content_getBuffer (nc);
        
        if (buf)
        {
          printf("'%s'\n", buf->buffer);
        }
        
        
        nc = echttp_content_next (nc);
      }
      
      ecmultipartparser_destroy (&mp);
    }    
  }
  
  return 0;
}
