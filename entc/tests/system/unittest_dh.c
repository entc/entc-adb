#include <utils/eclogger.h>
#include <system/ecfile.h>

int main (int argc, char *argv[])
{
  EcLogger logger = eclogger_new(0);

  EcDirHandle dh1 = ecdh_create (".");

  EcFileInfo info;

  while (ecdh_next (dh1, &info, TRUE))
  {
    printf ("item %s size %" PRIu64 "\n", info->name, info->size);
  }
  
  ecdh_destroy (&dh1);
  



  return 0;
}