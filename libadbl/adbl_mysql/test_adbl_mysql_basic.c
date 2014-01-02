#include <utils/eclogger.h>
#include "adbl_mysql.c"

int main (int argc, char *argv[])
{
  /* variables */
  void* connptr = 0;
  AdblConnectionProperties cp;
  int res = TRUE;
  EcLogger logger = eclogger_new(0);

  if( argc > 4 )
  {
    cp.host = argv[1];
    cp.file = 0;
    cp.port = 0;
    cp.schema = argv[2];
    cp.username = argv[3];  
    cp.password = argv[4];    
  }
  else
  {
    cp.host = "127.0.0.1";  
    cp.file = 0;
    cp.port = 0;
    cp.schema = "test";
    cp.username = "root";  
    cp.password = "root";    
  }
  
  connptr = adblmodule_dbconnect(&cp, logger );
  if( !connptr )
  {
    eclogger_log(logger, LL_ERROR, "TEST", "Error by connecting");
    res = FALSE;
  }
  
  adblmodule_dbdisconnect( connptr, logger );
  
  
  
  if( res )
  {
    eclogger_log(logger, LL_INFO, "TEST", "TEST SUCCEDED");    
  }
  else
  {
    eclogger_log(logger, LL_ERROR, "TEST", "TEST FAILED");        
  }
  
  eclogger_delete( &logger );
  
  return res;
}