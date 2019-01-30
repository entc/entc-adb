#include "adbl_mysql.h"
#include "adbl.h"

//-----------------------------------------------------------------------------

// cape includes
#include "sys/cape_err.h"
#include "stc/cape_udc.h"

//-----------------------------------------------------------------------------

int main (int argc, char *argv[])
{
  CapeErr err = cape_err_new ();
  
  CapeUdc properties = cape_udc_new (CAPE_UDC_NODE, NULL);
  
  cape_udc_add_s_cp (properties, "host", "127.0.0.1");
  cape_udc_add_s_cp (properties, "schema", "test");
  
  cape_udc_add_s_cp (properties, "user", "test");
  cape_udc_add_s_cp (properties, "pass", "test");
  
  {
    AdblPvdSession session = adbl_pvd_open (properties, err);

    if (session)
    {
      // delete
      {
        CapeUdc params = cape_udc_new (CAPE_UDC_NODE, NULL);
        
        
        cape_udc_add_n       (params, "id", 1);

        int res = adbl_pvd_del (session, "test_table01", &params, err);
        if (res)
        {
          printf ("ERROR DEL: %s\n", cape_err_text(err));
          
          
        }
        
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
        
        CapeUdc results = adbl_pvd_get (session, "test_table01", NULL, &columns, err);
        
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
        
        last_inserted_row = adbl_pvd_ins (session, "test_table01", &values, err);
        
        if (last_inserted_row <= 0)
        {
          printf ("ERROR: %s\n", cape_err_text(err));
          
          
        }
      }
      
      // update all
      {
        CapeUdc values = cape_udc_new (CAPE_UDC_NODE, NULL);
        
        // define the columns we want to insert
        cape_udc_add_n       (values, "fk01", 10);
        cape_udc_add_s_cp    (values, "col02", "is small");

        int res = adbl_pvd_set (session, "test_table01", NULL, &values, err);
        
      }
      
      // update last row
      {
        CapeUdc params = cape_udc_new (CAPE_UDC_NODE, NULL);
        
        
        cape_udc_add_n       (params, "id", last_inserted_row);
        
        CapeUdc values = cape_udc_new (CAPE_UDC_NODE, NULL);
        
        // define the columns we want to insert
        cape_udc_add_n       (values, "fk01", 10);
        cape_udc_add_s_cp    (values, "col02", "is last");
        
        int res = adbl_pvd_set (session, "test_table01", &params, &values, err);
        
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
        
        CapeUdc results = adbl_pvd_get (session, "test_table01", NULL, &columns, err);
        
        if (results)
        {
          cape_udc_del (&results);
        }
      }
      
      
      adbl_pvd_close (&session);
    } 
    else
    {
      printf ("ERROR: %s\n", cape_err_text(err));
    }
  }

  cape_udc_del (&properties);
  cape_err_del (&err);
  
  return 0;
}

//-----------------------------------------------------------------------------

