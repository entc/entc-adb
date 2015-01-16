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
    cp.password = "";    
  }
  
  connptr = adblmodule_dbconnect(&cp, logger );
  if( !connptr )
  {
    eclogger_log(logger, LL_ERROR, "TEST", "Error by connecting");
    res = FALSE;
  }
  
  
  AdblQuery* q1 = adbl_query_new ();
  
  adbl_query_setTable (q1, "test1");
  
  //adbl_query_addColumn (q1, "id", 0);
  adbl_query_addColumn (q1, "c1", 0);
  
  AdblConstraint* const1 = adbl_constraint_new (QUOMADBL_CONSTRAINT_AND);
  
  adbl_constraint_addChar (const1, "id", QUOMADBL_CONSTRAINT_EQUAL, "1");
  
  adbl_query_setConstraint (q1, const1);
  
  void* c1 = adblmodule_dbquery (connptr, q1, logger);
  if (c1)
  {
    while (adblmodule_dbcursor_next (c1))
    {
      eclogger_log(logger, LL_INFO, "TEST", adblmodule_dbcursor_data (c1, 0));    
     // eclogger_log(logger, LL_INFO, "TEST", adblmodule_dbcursor_data (c1, 1));    
    }    
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
  
  eclogger_del ( &logger );
  
  return res;
}