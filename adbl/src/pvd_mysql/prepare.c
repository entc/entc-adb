#include "prepare.h"

#include "adbl.h"

// cape includes
#include "stc/cape_stream.h"

//-----------------------------------------------------------------------------

struct AdblPrepare_s
{
  MYSQL_STMT* stmt;              // will be transfered
  
  number_t columns_used;
  
  number_t params_used; 
  
  CapeUdc params;                // owned
  
  CapeUdc values;                // will be transfered
  
  AdblBindVars bindsParams;     // owned
  
  AdblBindVars bindsValues;     // will be transfered
  
};

//-----------------------------------------------------------------------------

AdblPrepare adbl_prepare_new (MYSQL* mysql, CapeUdc* p_params, CapeUdc* p_values)
{
  AdblPrepare self = CAPE_NEW(struct AdblPrepare_s);
  
  self->stmt = mysql_stmt_init (mysql);
  
  self->values = NULL;
  self->params = NULL;
  
  // check all values
  if (p_values)
  {
    CapeUdc values = *p_values;
    
    self->values = cape_udc_new (CAPE_UDC_LIST, NULL);
    
    CapeUdcCursor* cursor = cape_udc_cursor_new (values, CAPE_DIRECTION_FORW);
    
    while (cape_udc_cursor_next (cursor))
    {
      CapeUdc item = cape_udc_cursor_ext (values, cursor);
      
      switch (cape_udc_type(item))
      {
        case CAPE_UDC_STRING:
        case CAPE_UDC_BOOL:
        case CAPE_UDC_FLOAT:
        {
          cape_udc_add (self->values, &item);
          break;
        }
        case CAPE_UDC_NUMBER:
        {
          // check the value
          number_t val = cape_udc_n (item, ADBL_AUTO_INCREMENT);
          
          if (val == ADBL_AUTO_INCREMENT)
          {
            cape_udc_del (&item);
          }
          else if (val == ADBL_AUTO_SEQUENCE_ID)
          {
            cape_udc_del (&item);
          }
          else
          {
            cape_udc_add (self->values, &item);
          }

          break;
        }
        default:
        {
          cape_udc_del (&item);
          break;
        }
      }
      
    }
    
    cape_udc_cursor_del (&cursor);
    
    cape_udc_del (p_values);
  }
    
  // params are optional
  if (p_params)
  {
    self->params = *p_params;
    *p_params = NULL;
  }
  
  self->columns_used = 0;
  self->params_used = 0;
  
  self->bindsValues = NULL;
  self->bindsParams = NULL;
  
  return self;
}

//-----------------------------------------------------------------------------

void adbl_prepare_del (AdblPrepare* p_self)
{
  AdblPrepare self = *p_self;
  
  if (self->params)
  {
    cape_udc_del (&(self->params));
  }
  
  if (self->values)
  {
    cape_udc_del (&(self->values));
  }
  
  if (self->bindsParams)
  {
    adbl_bindvars_del (&(self->bindsParams));    
  }
  
  if (self->bindsValues)
  {
    adbl_bindvars_del (&(self->bindsValues));    
  }
  
  if (self->stmt)
  {
    mysql_stmt_free_result (self->stmt);
    
    mysql_stmt_close (self->stmt);
  }
  
  CAPE_DEL(p_self, struct AdblPrepare_s);
}

//-----------------------------------------------------------------------------

AdblPvdCursor adbl_prepare_to_cursor (AdblPrepare* p_self)
{
  AdblPrepare self = *p_self;
  
  AdblPvdCursor cursor = CAPE_NEW(struct AdblPvdCursor_s);
  
  cursor->binds = self->bindsValues;
  self->bindsValues = NULL;
  
  cursor->stmt = self->stmt;
  self->stmt = NULL;
  
  cursor->values = self->values;
  self->values = NULL;
  
  cursor->pos = 0;
  
  // cleanup
  adbl_prepare_del (p_self);
  
  return cursor;  
}

//-----------------------------------------------------------------------------

int adbl_prepare_binds_params (AdblPrepare self, CapeErr err)
{
  int res;
  
  if (self->params_used)  // optional
  {
    // create bindings for mysql prepared statement engine
    self->bindsParams = adbl_bindvars_new (self->params_used);
    
    // set bindings for mysql for all parameters
    res = adbl_bindvars_set_from_node (self->bindsParams, self->params, err);
    if (res)
    {
      return res;
    }
    
    // try to bind all constraint values
    if (mysql_stmt_bind_param (self->stmt, adbl_bindvars_binds(self->bindsParams)) != 0)
    {
      return cape_err_set (err, CAPE_ERR_3RDPARTY_LIB, mysql_stmt_error (self->stmt));
    }
  }
  
  return CAPE_ERR_NONE;  
}

//-----------------------------------------------------------------------------

int adbl_prepare_binds_values (AdblPrepare self, CapeErr err)
{
  int res;
  
  if (self->columns_used)  // optional
  {
    // create bindings for mysql prepared statement engine
    self->bindsValues = adbl_bindvars_new (self->columns_used);
    
    // set bindings for mysql for all parameters
    res = adbl_bindvars_set_from_node (self->bindsValues, self->values, err);
    if (res)
    {
      return res;
    }
    
    // try to bind all constraint values
    if (mysql_stmt_bind_param (self->stmt, adbl_bindvars_binds(self->bindsValues)) != 0)
    {
      return cape_err_set (err, CAPE_ERR_3RDPARTY_LIB, mysql_stmt_error (self->stmt));
    }
  }
  
  return CAPE_ERR_NONE;  
}

//-----------------------------------------------------------------------------

int adbl_prepare_binds_result (AdblPrepare self, CapeErr err)
{
  int res;
  
  // allocate bind buffer
  self->bindsValues = adbl_bindvars_new (self->columns_used);
  
  res = adbl_bindvars_add_from_node (self->bindsValues, self->values, err);
  if (res)
  {
    return res;
  }
  
  // try to bind result
  if (mysql_stmt_bind_result (self->stmt, adbl_bindvars_binds(self->bindsValues)) != 0)
  {
    return cape_err_set (err, CAPE_ERR_3RDPARTY_LIB, mysql_stmt_error (self->stmt));
  }
  
  return CAPE_ERR_NONE;
}

//-----------------------------------------------------------------------------

int adbl_prepare_binds_all (AdblPrepare self, CapeErr err)
{
  int res;
  number_t size = self->columns_used + self->params_used;
  
  //printf ("HH: C = %li | P = %li\n", self->columns_used, self->params_used);
  
  // allocate bind buffer
  self->bindsValues = adbl_bindvars_new (size);

  if (self->columns_used)
  {
    res = adbl_bindvars_set_from_node (self->bindsValues, self->values, err);
    if (res)
    {
      return res;
    }
  }
  
  if (self->params_used)
  {
    res = adbl_bindvars_set_from_node (self->bindsValues, self->params, err);
    if (res)
    {
      return res;
    }
  }
  
  // try to bind result
  if (mysql_stmt_bind_param (self->stmt, adbl_bindvars_binds(self->bindsValues)) != 0)
  {
    return cape_err_set (err, CAPE_ERR_3RDPARTY_LIB, mysql_stmt_error (self->stmt));
  }
  
  return CAPE_ERR_NONE;  
}

//-----------------------------------------------------------------------------

number_t adbl_prepare_execute (AdblPrepare self, MYSQL* mysql, CapeErr err)
{
  // execute
  if (mysql_stmt_execute (self->stmt) != 0)
  {
    cape_err_set (err, CAPE_ERR_3RDPARTY_LIB, mysql_stmt_error (self->stmt));
    
    return -1;
  }
  
  if (mysql_stmt_store_result (self->stmt) != 0)
  {
    cape_err_set (err, CAPE_ERR_3RDPARTY_LIB, mysql_stmt_error (self->stmt));
    
    return -1;
  }
  
  number_t last_insert_id = mysql_insert_id (mysql);

  // printf ("LAST INSERT ID: %li\n", last_insert_id);
  
  return last_insert_id;
}

//-----------------------------------------------------------------------------

number_t adbl_pvd_append_columns (CapeStream stream, int ansi, CapeUdc values, const char* table)
{
  number_t used = 0;
  CapeUdcCursor* cursor = cape_udc_cursor_new (values, CAPE_DIRECTION_FORW);
  
  while (cape_udc_cursor_next (cursor))
  {
    const CapeString column_name = cape_udc_name (cursor->item);
    
    if (column_name)
    {
      if (cursor->position > 0)
      {
        cape_stream_append_str (stream, ", ");
      }
      
      if (ansi == TRUE)
      {
        cape_stream_append_str (stream, "\"" );
        cape_stream_append_str (stream, table );
        cape_stream_append_str (stream, "\".\"" );
        cape_stream_append_str (stream, column_name);
        cape_stream_append_str (stream, "\"" );
      }
      else
      {
        cape_stream_append_str (stream, table );
        cape_stream_append_str (stream, "." );
        cape_stream_append_str (stream, column_name);
      }
      
      used++;
    }
  }
  
  cape_udc_cursor_del (&cursor);
  
  return used;
}

//-----------------------------------------------------------------------------

number_t adbl_pvd_append_update (CapeStream stream, int ansi, CapeUdc values, const char* table)
{
  number_t used = 0;
  CapeUdcCursor* cursor = cape_udc_cursor_new (values, CAPE_DIRECTION_FORW);
  
  while (cape_udc_cursor_next (cursor))
  {
    const CapeString column_name = cape_udc_name (cursor->item);
    
    if (column_name)
    {
      if (cursor->position > 0)
      {
        cape_stream_append_str (stream, ", ");
      }
      
      if (ansi == TRUE)
      {
        cape_stream_append_str (stream, "\"" );
        cape_stream_append_str (stream, table );
        cape_stream_append_str (stream, "\".\"" );
        cape_stream_append_str (stream, column_name);
        cape_stream_append_str (stream, "\" = ?" );
      }
      else
      {
        cape_stream_append_str (stream, table );
        cape_stream_append_str (stream, "." );
        cape_stream_append_str (stream, column_name);
        cape_stream_append_str (stream, " = ?" );
      }
      
      used++;
    }
  }
  
  cape_udc_cursor_del (&cursor);
  
  return used;
}

//-----------------------------------------------------------------------------

number_t adbl_prepare_append_values (CapeStream stream, CapeUdc values)
{
  number_t used = 0;
  
  CapeUdcCursor* cursor = cape_udc_cursor_new (values, CAPE_DIRECTION_FORW);
  
  while (cape_udc_cursor_next (cursor))
  {
    const CapeString param_name = cape_udc_name (cursor->item);
    
    if (param_name)
    {
      if (cursor->position > 0)
      {
        cape_stream_append_str (stream, ", ");
      }
      
      cape_stream_append_str (stream, "?" );
      
      used++;
    }
  }
  
  cape_udc_cursor_del (&cursor);
  
  return used;
}

//-----------------------------------------------------------------------------

number_t adbl_prepare_append_constraints (CapeStream stream, int ansi, CapeUdc params, const char* table)
{
  number_t used = 0;

  CapeUdcCursor* cursor = cape_udc_cursor_new (params, CAPE_DIRECTION_FORW);
  
  while (cape_udc_cursor_next (cursor))
  {
    const CapeString param_name = cape_udc_name (cursor->item);
    
    if (param_name)
    {
      if (cursor->position > 0)
      {
        cape_stream_append_str (stream, " AND ");
      }
      
      if (ansi == TRUE)
      {
        cape_stream_append_str (stream, "\"" );
        cape_stream_append_str (stream, table );
        cape_stream_append_str (stream, "\".\"" );
        cape_stream_append_str (stream, param_name);
        cape_stream_append_str (stream, "\" = ?" );
      }
      else
      {
        cape_stream_append_str (stream, table );
        cape_stream_append_str (stream, "." );
        cape_stream_append_str (stream, param_name);   
        cape_stream_append_str (stream, " = ?" );
      }
      
      used++;
    }
  }
  
  cape_udc_cursor_del (&cursor);
  
  return used;
}

//-----------------------------------------------------------------------------

number_t adbl_prepare_append_where_clause (CapeStream stream, int ansi, CapeUdc params, const char* table)
{
  if (params == NULL)
  {
    return 0;
  }
  
  cape_stream_append_str (stream, " WHERE ");
  
  return adbl_prepare_append_constraints (stream, ansi, params, table);
}

//-----------------------------------------------------------------------------

void adbl_pvd_append_table (CapeStream stream, int ansi, const char* schema, const char* table)
{
  // schema and table name
  if (ansi == TRUE)
  {
    cape_stream_append_str (stream, "\"" );
    cape_stream_append_str (stream, schema );
    cape_stream_append_str (stream, "\".\"" );
    cape_stream_append_str (stream, table );
    cape_stream_append_str (stream, "\"" );
  }
  else
  {
    cape_stream_append_str (stream, schema );
    cape_stream_append_str (stream, "." );
    cape_stream_append_str (stream, table );
  }
}

//-----------------------------------------------------------------------------

int adbl_prepare_statement_select (AdblPrepare self, const char* schema, const char* table, int ansi, CapeErr err)
{
  int res;
  
  CapeStream stream = cape_stream_new ();
  CapeString sql_statement = NULL;
  
  cape_stream_append_str (stream, "SELECT ");
  
  // create columns for mysql for all parameters
  self->columns_used = adbl_pvd_append_columns (stream, ansi, self->values, table);
  
  cape_stream_append_str (stream, " FROM ");
  
  adbl_pvd_append_table (stream, ansi, schema, table);
  
  self->params_used = adbl_prepare_append_where_clause (stream, ansi, self->params, table);
  
  {
    number_t sql_size = cape_stream_size (stream);
    
    sql_statement = cape_stream_to_str (&stream);
    
    // printf ("   ** SQL ** %s\n", sql_statement);
    
    // prepare the statement 
    if (mysql_stmt_prepare (self->stmt, sql_statement, sql_size) != 0)
    {
      res = cape_err_set (err, CAPE_ERR_3RDPARTY_LIB, mysql_stmt_error (self->stmt));
      goto exit_and_cleanup;
    }
  }
  
  res = CAPE_ERR_NONE;
  
  // --------------
  exit_and_cleanup:
  
  if (stream)
  {
    cape_stream_del (&stream);
  }
  
  if (sql_statement)
  {
    cape_str_del (&sql_statement);
  }
  
  return res;
}

//-----------------------------------------------------------------------------

int adbl_prepare_statement_insert (AdblPrepare self, const char* schema, const char* table, int ansi, CapeErr err)
{
  int res;
  
  CapeStream stream = cape_stream_new ();
  CapeString sql_statement = NULL;

  cape_stream_append_str (stream, "INSERT INTO ");
  
  adbl_pvd_append_table (stream, ansi, schema, table);

  cape_stream_append_str (stream, " (");
  
  // create columns for mysql for all parameters
  self->columns_used = adbl_pvd_append_columns (stream, ansi, self->values, table);
  
  cape_stream_append_str (stream, ") VALUES (");

  adbl_prepare_append_values (stream, self->values);

  cape_stream_append_str (stream, ")");
  
  {
    number_t sql_size = cape_stream_size (stream);
    
    sql_statement = cape_stream_to_str (&stream);
    
    //printf ("   ** SQL ** %s\n", sql_statement);
    
    // prepare the statement 
    if (mysql_stmt_prepare (self->stmt, sql_statement, sql_size) != 0)
    {
      res = cape_err_set (err, CAPE_ERR_3RDPARTY_LIB, mysql_stmt_error (self->stmt));
      goto exit_and_cleanup;
    }
  }
  
  res = CAPE_ERR_NONE;

  // --------------
  exit_and_cleanup:
  
  if (stream)
  {
    cape_stream_del (&stream);
  }
  
  if (sql_statement)
  {
    cape_str_del (&sql_statement);
  }
  
  return res;
}

//-----------------------------------------------------------------------------

int adbl_prepare_statement_delete (AdblPrepare self, const char* schema, const char* table, int ansi, CapeErr err)
{
  int res;
  
  CapeStream stream = cape_stream_new ();
  CapeString sql_statement = NULL;

  cape_stream_append_str (stream, "DELETE FROM ");

  adbl_pvd_append_table (stream, ansi, schema, table);

  self->params_used = adbl_prepare_append_where_clause (stream, ansi, self->params, table);
  
  {
    number_t sql_size = cape_stream_size (stream);
    
    sql_statement = cape_stream_to_str (&stream);
    
    printf ("   ** SQL ** %s\n", sql_statement);
    
    // prepare the statement 
    if (mysql_stmt_prepare (self->stmt, sql_statement, sql_size) != 0)
    {
      res = cape_err_set (err, CAPE_ERR_3RDPARTY_LIB, mysql_stmt_error (self->stmt));
      goto exit_and_cleanup;
    }
  }
  
  res = CAPE_ERR_NONE;
  
  // --------------
  exit_and_cleanup:
  
  if (stream)
  {
    cape_stream_del (&stream);
  }
  
  if (sql_statement)
  {
    cape_str_del (&sql_statement);
  }
  
  return res;
}

//-----------------------------------------------------------------------------

int adbl_prepare_statement_update (AdblPrepare self, const char* schema, const char* table, int ansi, CapeErr err)
{
  int res;
  
  CapeStream stream = cape_stream_new ();
  CapeString sql_statement = NULL;
  
  cape_stream_append_str (stream, "UPDATE ");
  
  adbl_pvd_append_table (stream, ansi, schema, table);
  
  cape_stream_append_str (stream, " SET ");

  self->columns_used = adbl_pvd_append_update (stream, ansi, self->values, table);
  
  self->params_used = adbl_prepare_append_where_clause (stream, ansi, self->params, table);
  
  {
    number_t sql_size = cape_stream_size (stream);
    
    sql_statement = cape_stream_to_str (&stream);
    
    printf ("   ** SQL ** %s\n", sql_statement);
    
    // prepare the statement 
    if (mysql_stmt_prepare (self->stmt, sql_statement, sql_size) != 0)
    {
      res = cape_err_set (err, CAPE_ERR_3RDPARTY_LIB, mysql_stmt_error (self->stmt));
      goto exit_and_cleanup;
    }
  }
  
  res = CAPE_ERR_NONE;
  
  // --------------
  exit_and_cleanup:
  
  if (stream)
  {
    cape_stream_del (&stream);
  }
  
  if (sql_statement)
  {
    cape_str_del (&sql_statement);
  }
  
  return res;
}

//-----------------------------------------------------------------------------

int adbl_prepare_statement_setins (AdblPrepare self, const char* schema, const char* table, int ansi, CapeErr err)
{
  int res;
  
  CapeStream stream = cape_stream_new ();
  CapeString sql_statement = NULL;
  
  cape_stream_append_str (stream, "INSERT INTO ");
  
  adbl_pvd_append_table (stream, ansi, schema, table);
  
  cape_stream_append_str (stream, " (");
  
  // create columns for mysql for all parameters
  self->columns_used = adbl_pvd_append_columns (stream, ansi, self->values, table);
  
  cape_stream_append_str (stream, ") VALUES (");
  
  adbl_prepare_append_values (stream, self->values);
  
  cape_stream_append_str (stream, ")");
  
  cape_stream_append_str (stream, " ON DUPLICATE KEY UPDATE ");
  
  self->columns_used = adbl_pvd_append_update (stream, ansi, self->values, table);
  
  {
    number_t sql_size = cape_stream_size (stream);
    
    sql_statement = cape_stream_to_str (&stream);
    
    printf ("   ** SQL ** %s\n", sql_statement);
    
    // prepare the statement 
    if (mysql_stmt_prepare (self->stmt, sql_statement, sql_size) != 0)
    {
      res = cape_err_set (err, CAPE_ERR_3RDPARTY_LIB, mysql_stmt_error (self->stmt));
      goto exit_and_cleanup;
    }
  }
  
  res = CAPE_ERR_NONE;
  
  // --------------
  exit_and_cleanup:
  
  if (stream)
  {
    cape_stream_del (&stream);
  }
  
  if (sql_statement)
  {
    cape_str_del (&sql_statement);
  }
  
  return res;  
}

//-----------------------------------------------------------------------------


