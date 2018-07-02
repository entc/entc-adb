#include "adbo2.h"

//=============================================================================

int main(int argc, char* argv[])
{
  EcString path = ecfs_getCurrentDirectory();
  
  Adbo2 adbo = adbo2_create (path, path, "adbo");

  Adbo2Context ctx = adbo2_ctx (adbo, NULL, NULL);
  if (ctx)
  {
    
    
    adbo2_ctx_destroy (&ctx);
  }
    
  adbo2_destroy (&adbo);

  ecstr_delete (&path);
}

//-----------------------------------------------------------------------------

