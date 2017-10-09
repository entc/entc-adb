// c includes
#include <stdio.h>
#include <stdlib.h>

// entc includes
#include "system/ecdlib.h"
#include "types/ecmap.h"
#include "types/ecstream.h"
#include "tests/eclua.h"

//---------------------------------------------------------------------------

int main_run_script (EcLua lua, const EcString script, EcErr err)
{
  int res;
  
  res = eclua_init (lua, err);
  if (res)
  {
    printf("ERROR: %s\n", err->text);
    return 1;
  }
  
  res = eclua_run (lua, script, err);
  if (res)
  {
    printf("ERROR: %s\n", err->text);
    return 2;
  }

  return 0;
}

//---------------------------------------------------------------------------

int main(int argc, char* argv[])
{
  int res;
  EcErr err;
  EcLua lua;
  
  if (argc < 2)
  {
    printf ("no script specified\n");
    return 1;
  }

  err = ecerr_create ();
  lua = eclua_create ();

  res = main_run_script (lua, argv[1], err);
  
  eclua_destroy (&lua);
  ecerr_destroy (&err);
  
  return res;
}

//---------------------------------------------------------------------------
