#include <utils/eclogger.h>
#include "adbl_mysql.c"

int main (int argc, char *argv[])
{
  ecmessages_initialize ();
  
  /* variables */
  void* connptr = 0;
  AdblConnectionProperties cp;
  int res = TRUE;

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
    cp.host = "localhost";
    cp.file = 0;
    cp.port = 0;
    cp.schema = "test1";
    cp.username = "root";  
    cp.password = "lobo2020";
  }
  
  connptr = adblmodule_dbconnect (&cp);
  if( !connptr )
  {
    eclogger_msg (LL_ERROR, "TEST", "basic", "Error by connecting");
    exit (1);
  }
  else
  {
    eclogger_msg (LL_INFO, "TEST", "basic", "connected");
  }
  
  
  AdblQuery* q1 = adbl_query_new ();
  
  adbl_query_setTable (q1, "test1");
  
  //adbl_query_addColumn (q1, "id", 0);
  adbl_query_addColumn (q1, "c1", 0);
  
  AdblConstraint* const1 = adbl_constraint_new (QUOMADBL_CONSTRAINT_AND);
  
  adbl_constraint_addChar (const1, "id", QUOMADBL_CONSTRAINT_EQUAL, "2");
  
  adbl_query_setConstraint (q1, const1);
  
  void* c1 = adblmodule_dbquery (connptr, q1);
  if (c1)
  {
    while (adblmodule_dbcursor_next (c1))
    {
      eclogger_msg (LL_INFO, "TEST", "basic", adblmodule_dbcursor_data (c1, 0));    
     // eclogger_log(logger, LL_INFO, "TEST", adblmodule_dbcursor_data (c1, 1));    
    }    
  }
  
  
  
  adblmodule_dbdisconnect (connptr);
  
  
  
  if( res )
  {
    eclogger_msg (LL_INFO, "TEST", "basic", "TEST SUCCEDED");    
  }
  else
  {
    eclogger_msg (LL_ERROR, "TEST", "basic", "TEST FAILED");        
  }
  
  return res;
}
