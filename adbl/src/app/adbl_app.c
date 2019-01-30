#include "adbl.h"

#include <stc/cape_udc.h>

//-----------------------------------------------------------------------------

#include <stdio.h>

//-----------------------------------------------------------------------------

int main (int argc, char *argv[])
{
  CapeErr err = cape_err_new ();
  
  AdblCtx ctx = NULL;
  AdblSession session = NULL;
  AdblTrx trx = NULL;
  
  ctx = adbl_ctx_new ("pvd_mysql", "adbl_mysql", err);
  if (ctx == NULL)
  {
    goto exit;
  }
  
  
  {
    CapeUdc properties = cape_udc_new (CAPE_UDC_NODE, NULL);
    
    cape_udc_add_s_cp (properties, "host", "127.0.0.1");
    cape_udc_add_s_cp (properties, "schema", "test");
    
    cape_udc_add_s_cp (properties, "user", "test");
    cape_udc_add_s_cp (properties, "pass", "test");

    session = adbl_session_open (ctx, properties, err);
    
    cape_udc_del (&properties);
    
    if (session == NULL)
    {
      printf ("can't open session\n");
      
      goto exit;
    }
  }
  
  trx = adbl_trx_new (session, err);
  if (trx == NULL)
  {
    goto exit;
  }
  
  // fetch
  {
    //CapeUdc params = cape_udc_new (CAPE_UDC_NODE, NULL);
    
    
    //cape_udc_add_n       (params, "id", 1);
    
    CapeUdc columns = cape_udc_new (CAPE_UDC_NODE, NULL);
    
    
    // define the columns we want to fetch
    cape_udc_add_n       (columns, "fk01", 0);
    cape_udc_add_s_cp    (columns, "col01", NULL);
    cape_udc_add_s_cp    (columns, "col02", NULL);
    
    CapeUdc results = adbl_trx_query (trx, "test_table01", NULL, &columns, err);
    
    if (results)
    {
      cape_udc_del (&results);
    }
  }
  
  int i;
  number_t last_inserted_row = 0;
  
  // insert
  for (i = 0; i < 10; i++)
  {
    CapeUdc values = cape_udc_new (CAPE_UDC_NODE, NULL);
    
    // define the columns we want to insert
    cape_udc_add_n       (values, "id", ADBL_AUTO_INCREMENT);   // this column is an auto increment column
    cape_udc_add_n       (values, "fk01", 42);
    cape_udc_add_s_cp    (values, "col01", "my column");
    cape_udc_add_s_cp    (values, "col02", "is great");
    
    last_inserted_row = adbl_trx_insert (trx, "test_table01", &values, err);
    
    if (last_inserted_row <= 0)
    {
      printf ("ERROR: %s\n", cape_err_text(err));
      
      
    }
  }
  
  adbl_trx_commit (&trx, err);
  
exit:

  if (trx)
  {
    adbl_trx_rollback (&trx, err);
  }

  if (cape_err_code(err))
  {
    printf ("ERROR: %s\n", cape_err_text(err));
  }
  
  if (session)
  {
    adbl_session_close (&session);
  }
  
  if (ctx)
  {
    adbl_ctx_del (&ctx);
  }
  
  cape_err_del (&err);
  
  return 0;
}

//-----------------------------------------------------------------------------

