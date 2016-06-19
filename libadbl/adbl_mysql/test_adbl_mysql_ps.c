#include <utils/eclogger.h>
#include "adbl_mysql.c"

int main (int argc, char *argv[])
{
  ecmessages_initialize ();

  void* connptr = 0;
  AdblConnectionProperties cp;
  int res = TRUE;

  cp.host = "127.0.0.1";  
  cp.file = 0;
  cp.port = 3306;
  cp.schema = "lobo_qnif_01";
  cp.username = "qnif2";  
  cp.password = "88gdQW878kj";    
  
  connptr = adblmodule_dbconnect (&cp);
  if( !connptr )
  {
    eclogger_msg (LL_ERROR, "TEST", "ps", "Error by connecting");
    res = FALSE;
  }
  
  AdblQuery* q1 = adbl_query_new ();
  
  adbl_query_setTable (q1, "users");
  
  adbl_query_addColumn (q1, "id", 0);
  adbl_query_addColumn (q1, "name", 0);
  adbl_query_addColumn (q1, "admin", 0);
  adbl_query_addColumn (q1, "secret", 0);
  adbl_query_addColumn (q1, "user512", 0);
  
  AdblConstraint* const1 = adbl_constraint_new (QUOMADBL_CONSTRAINT_AND);
  
  adbl_constraint_addChar (const1, "user512", QUOMADBL_CONSTRAINT_EQUAL, "4135aa9dc1b842a653dea846903ddb95bfb8c5a10c504a7fa16e10bc31d1fdf0");
  adbl_constraint_addLong (const1, "admin", QUOMADBL_CONSTRAINT_EQUAL, 1);
  
  adbl_query_setConstraint (q1, const1);
  
  void* c1 = adblmodule_dbquery (connptr, q1);
  if (c1)
  {
    while (adblmodule_dbcursor_next (c1))
    {
      eclogger_msg (LL_INFO, "TEST", "basic", adblmodule_dbcursor_data (c1, 0));    
      eclogger_msg (LL_INFO, "TEST", "basic", adblmodule_dbcursor_data (c1, 1));    
      eclogger_msg (LL_INFO, "TEST", "basic", adblmodule_dbcursor_data (c1, 2));    
      eclogger_msg (LL_INFO, "TEST", "basic", adblmodule_dbcursor_data (c1, 3));    
      eclogger_msg (LL_INFO, "TEST", "basic", adblmodule_dbcursor_data (c1, 4));    
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
  
  ecmessages_deinitialize ();
  
  return res;
}