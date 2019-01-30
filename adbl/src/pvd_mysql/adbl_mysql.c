#include "adbl_mysql.h"

#include "bindvars.h"
#include "prepare.h"

// cape includes
#include "sys/cape_types.h"
#include "stc/cape_stream.h"

//-----------------------------------------------------------------------------

#include <mysql.h>

static int init_status = 0;

//------------------------------------------------------------------------------------------------------

void __attribute__ ((constructor)) library_init (void)
{
  printf ("MYSQLClient: INIT\n");
  
  mysql_library_init (0, NULL, NULL);  
}

//------------------------------------------------------------------------------------------------------

void __attribute__ ((destructor)) library_fini (void)
{
  printf ("MYSQLClient: DONE\n");

  mysql_thread_end ();
  
  mysql_library_end ();
}

//------------------------------------------------------------------------------------------------------

void adbl_mysql_init ()
{
  if (init_status == 0)
  {
    printf ("MYSQLClient: INIT\n");

    mysql_library_init (0, NULL, NULL);
  }
  
  init_status++;
}

//------------------------------------------------------------------------------------------------------

void adbl_mysql_done ()
{
  init_status--;
  
  if (init_status == 0)
  {
    printf ("MYSQLClient: DONE\n");
    
    mysql_thread_end ();

    mysql_library_end ();    
  }
}

//-----------------------------------------------------------------------------

struct AdblPvdSession_s
{
  MYSQL* mysql;      // mysql clientlibrary handle
  
  char* schema;      // the schema used for statements
  
  int ansi_quotes;   // defines if the staments must be quoted
  
};

//-----------------------------------------------------------------------------

AdblPvdSession __STDCALL adbl_pvd_open (CapeUdc cp, CapeErr err)
{
  MYSQL_RES* res;
  
  AdblPvdSession self = CAPE_NEW(struct AdblPvdSession_s);
  
  self->ansi_quotes = FALSE;
  
  // call this before mysql_init
  //adbl_mysql_init ();
  
  // init mysql
  self->mysql = mysql_init (NULL);
  
  self->schema = cape_str_cp (cape_udc_get_s (cp, "schema", NULL));
  
  
  // settings
  mysql_options (self->mysql, MYSQL_OPT_COMPRESS, 0);
  
  my_bool reconnect = TRUE;
  mysql_options (self->mysql, MYSQL_OPT_RECONNECT, &reconnect);
  
  // we start with no transaction -> activate autocommit
  mysql_options (self->mysql, MYSQL_INIT_COMMAND, "SET autocommit=1");
  
  // connect
  if(!mysql_real_connect (self->mysql, cape_udc_get_s (cp, "host", "127.0.0.1"), cape_udc_get_s (cp, "user", "admin"), cape_udc_get_s (cp, "pass", "admin"), self->schema, cape_udc_get_n (cp, "port", 3306), 0, CLIENT_MULTI_RESULTS))
  {    
    cape_err_set (err, CAPE_ERR_3RDPARTY_LIB, mysql_error (self->mysql));
    
    //cape_log_msg (LL_ERROR, "ADBL", "mysql", mysql_error (self->conn));

    adbl_pvd_close (&self);
    return NULL;
  }
  
  // find out the ansi variables
  mysql_query (self->mysql, "SELECT @@global.sql_mode");
  res = mysql_use_result (self->mysql);
  if(res)
  {
    MYSQL_ROW row;
    row = mysql_fetch_row (res);
    
    self->ansi_quotes = strstr (row[0], "ANSI_QUOTES" ) != 0;
    
    mysql_free_result(res);
  }
  
  return self;
}

//-----------------------------------------------------------------------------

void __STDCALL adbl_pvd_close (AdblPvdSession* p_self)
{
  AdblPvdSession self = *p_self;
  
  printf ("MYSQL: close session\n");
  
  cape_str_del (&(self->schema));
  
  
  
  mysql_close (self->mysql);
  
  CAPE_DEL(p_self, struct AdblPvdSession_s);
  
  // call this after mysql_close
  //adbl_mysql_done ();
}

//-----------------------------------------------------------------------------

CapeUdc __STDCALL adbl_pvd_get (AdblPvdSession self, const char* table, CapeUdc* p_params, CapeUdc* p_values, CapeErr err)
{
  AdblPvdCursor cursor = adbl_pvd_cursor_new (self, table, p_params, p_values, err);
  
  if (cursor == NULL)
  {
    printf ("ERROR: %s\n", cape_err_text(err));
    
    return NULL;
  }
  
  {
    CapeUdc results = cape_udc_new (CAPE_UDC_LIST, NULL);

    {
      // fetch all rows and merge them intoone result set  
      while (adbl_pvd_cursor_next (cursor))
      {
        // this will transfer the ownership
        CapeUdc result_row = adbl_pvd_cursor_get (cursor);
        
        // add this rowto the list
        cape_udc_add (results, &result_row);
      }
      
      adbl_pvd_cursor_del (&cursor);
    }
        
    return results;
  }
}

//-----------------------------------------------------------------------------

number_t __STDCALL adbl_pvd_ins (AdblPvdSession self, const char* table, CapeUdc* p_values, CapeErr err)
{
  int res;
  number_t last_insert_id = 0;
  
  AdblPrepare pre = adbl_prepare_new (self->mysql, NULL, p_values);

  res = adbl_prepare_statement_insert (pre, self->schema, table, self->ansi_quotes, err);
  if (res)
  {
    printf ("WARN: %s\n", cape_err_text(err));
    
    goto exit_and_cleanup;
  }
  
  res = adbl_prepare_binds_values (pre, err);
  if (res)
  {
    printf ("WARN: %s\n", cape_err_text(err));

    goto exit_and_cleanup;
  }
  
  last_insert_id = adbl_prepare_execute (pre, self->mysql, err);
  
  // --------------
exit_and_cleanup:
  
  adbl_prepare_del (&pre);
  
  return last_insert_id;
}

//-----------------------------------------------------------------------------

int __STDCALL adbl_pvd_del (AdblPvdSession self, const char* table, CapeUdc* p_params, CapeErr err)
{
  int res;
  number_t last_insert_id = 0;
  
  AdblPrepare pre = adbl_prepare_new (self->mysql, p_params, NULL);
  
  res = adbl_prepare_statement_delete (pre, self->schema, table, self->ansi_quotes, err);
  if (res)
  {
    printf ("WARN: %s\n", cape_err_text(err));
    
    goto exit_and_cleanup;
  }
  
  res = adbl_prepare_binds_params (pre, err);
  if (res)
  {
    goto exit_and_cleanup;    
  }
    
  last_insert_id = adbl_prepare_execute (pre, self->mysql, err);
  
// --------------
exit_and_cleanup:
  
  adbl_prepare_del (&pre);
  
  return last_insert_id;
}

//-----------------------------------------------------------------------------

int __STDCALL adbl_pvd_set (AdblPvdSession self, const char* table, CapeUdc* p_params, CapeUdc* p_values, CapeErr err)
{
  int res;
  number_t last_insert_id = 0;
  
  AdblPrepare pre = adbl_prepare_new (self->mysql, p_params, p_values);
  
  res = adbl_prepare_statement_update (pre, self->schema, table, self->ansi_quotes, err);
  if (res)
  {
    printf ("WARN: %s\n", cape_err_text(err));
    
    goto exit_and_cleanup;
  }
  
  // all binds are done as parameter
  res = adbl_prepare_binds_all (pre, err);
  if (res)
  {
    printf ("WARN: %s\n", cape_err_text(err));

    goto exit_and_cleanup;    
  }
  
  last_insert_id = adbl_prepare_execute (pre, self->mysql, err);
  
// --------------
exit_and_cleanup:
  
  adbl_prepare_del (&pre);
  
  return last_insert_id;  
}

//-----------------------------------------------------------------------------

number_t __STDCALL adbl_pvd_ins_or_set (AdblPvdSession self, const char* table, CapeUdc* p_params, CapeUdc* p_values, CapeErr err)
{
  
}

//-----------------------------------------------------------------------------

int __STDCALL adbl_pvd_begin (AdblPvdSession self, CapeErr err)
{
  mysql_query (self->mysql, "START TRANSACTION");
  
  if (mysql_errno (self->mysql))
  {
    return cape_err_set (err, CAPE_ERR_3RDPARTY_LIB, mysql_error (self->mysql));
  }
  
  return CAPE_ERR_NONE;
}

//-----------------------------------------------------------------------------

int __STDCALL adbl_pvd_commit (AdblPvdSession self, CapeErr err)
{
  mysql_query (self->mysql, "COMMIT");
  
  if (mysql_errno (self->mysql))
  {
    return cape_err_set (err, CAPE_ERR_3RDPARTY_LIB, mysql_error (self->mysql));
  }
  
  return CAPE_ERR_NONE;
}

//-----------------------------------------------------------------------------

int __STDCALL adbl_pvd_rollback (AdblPvdSession self, CapeErr err)
{
  mysql_query (self->mysql, "ROLLBACK");
  
  if (mysql_errno (self->mysql))
  {
    return cape_err_set (err, CAPE_ERR_3RDPARTY_LIB, mysql_error (self->mysql));
  }
  
  return CAPE_ERR_NONE;
}

//-----------------------------------------------------------------------------

AdblPvdCursor __STDCALL adbl_pvd_cursor_new (AdblPvdSession session, const char* table, CapeUdc* p_params, CapeUdc* p_values, CapeErr err)
{
  int res;

  AdblPrepare pre = adbl_prepare_new (session->mysql, p_params, p_values);
  
  res = adbl_prepare_statement_select (pre, session->schema, table, session->ansi_quotes, err);
  if (res)
  {
    goto exit_and_cleanup;
  }
  
  res = adbl_prepare_binds_params (pre, err);
  if (res)
  {
    goto exit_and_cleanup;    
  }
  
  res = adbl_prepare_binds_result (pre, err);
  if (res)
  {
    goto exit_and_cleanup;
  }
  
  adbl_prepare_execute (pre, session->mysql, err);
  if (cape_err_code(err))
  {
    goto exit_and_cleanup;    
  }
  
  return adbl_prepare_to_cursor (&pre);
  
// --------------
exit_and_cleanup:
  
  adbl_prepare_del (&pre);
  
  return NULL;
}

//-----------------------------------------------------------------------------

void __STDCALL adbl_pvd_cursor_del (AdblPvdCursor* p_self)
{
  int i;
  AdblPvdCursor self = *p_self;

  mysql_stmt_free_result (self->stmt);
  
  mysql_stmt_close (self->stmt);
  
  // clean up the array
  adbl_bindvars_del (&(self->binds));   
  
  cape_udc_del (&(self->values));
  
  CAPE_DEL (p_self, struct AdblPvdCursor_s);
}

//-----------------------------------------------------------------------------

int __STDCALL adbl_pvd_cursor_next (AdblPvdCursor self)
{
  switch (mysql_stmt_fetch (self->stmt))
  {
    case 0:
    {
      return TRUE;
    }
    case MYSQL_NO_DATA:
    {
      return FALSE;
    }
    case 1:   // some kind of error happened
    {
      printf ("ERROR: %s\n", mysql_stmt_error(self->stmt));
      
      return FALSE;
    }
    case MYSQL_DATA_TRUNCATED:  // the data in the column don't fits into the binded buffer (1024)
    {
      printf ("WARNING: %s\n", "data truncated");
      
      return TRUE;
    }
  }
  
  return TRUE;
}

//-----------------------------------------------------------------------------

CapeUdc __STDCALL adbl_pvd_cursor_get (AdblPvdCursor self)
{
  CapeUdc result_row = cape_udc_new (CAPE_UDC_NODE, NULL);
  
  {
    int i = 0;
    CapeUdcCursor* cursor = cape_udc_cursor_new (self->values, CAPE_DIRECTION_FORW);
    
    while (cape_udc_cursor_next (cursor))
    {
      const CapeString column_name = cape_udc_name (cursor->item);
      
      if (column_name)
      {
        CapeUdc item = cape_udc_new (cape_udc_type (cursor->item), column_name);
        
        adbl_bindvars_get (self->binds, i, item);

        //cape_udc_print (item);
        
        cape_udc_add (result_row, &item);
        
        i++;
      }
    }
    
    cape_udc_cursor_del (&cursor);
  }
  
  return result_row;
}

//-----------------------------------------------------------------------------
