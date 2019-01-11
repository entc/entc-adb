#include <system/ecfile.h>
#include <system/ecmutex.h>
#include <types/ecstream.h>
#include <types/eclist.h>
#include <tools/eclog.h>

#include <mysql.h>
#include "adbl.h"

#define C_MODDESC "MYSQL"

//================================================================================================

#define MODULE "MYSQ"

static const AdblModuleInfo ModuleInfo = { 10000, MODULE, "Mysql" };

#include "adbl_module.inc"

//================================================================================================

struct AdblMysqlConnection_s
{
  
  MYSQL* conn;
  
  char* schema;
  
  int ansi;
  
  EcMutex mutex;
  
};

typedef struct AdblMysqlConnection_s* AdblMysqlConnection;

typedef struct {

  MYSQL_STMT* stmt;
  
  uint_t pos;
  
  uint_t size;
  
  // the followings are arrays with the same length
    
  MYSQL_BIND* bindResult;
  
  char* data;

  unsigned long* length;
  
  my_bool* is_null;
  
  my_bool* error;
  
} AdblMysqlCursor;

typedef struct 
{
  
  char* table;
  
  char* column;
  
  AdblMysqlConnection mysql;
  
} AdblMyslSequence;

//------------------------------------------------------------------------------------------------------

typedef struct
{
  
  MYSQL_BIND* binds;
  
  int max;
  
  int pos;
  
} AdblMysqlBindVars;

//------------------------------------------------------------------------------------------------------

static int init_status = 0;

//------------------------------------------------------------------------------------------------------

void adblmodule_init ()
{
  if (init_status == 0)
  {
    mysql_library_init (0, NULL, NULL);  
  }
  
  init_status++;
}

//------------------------------------------------------------------------------------------------------

void adblmodule_done ()
{
  init_status--;

  if (init_status == 0)
  {
    mysql_library_end ();  
  }
}

//------------------------------------------------------------------------------------------------------

AdblMysqlBindVars* bindvars_create (int bindCnt)
{
  ulong_t bindSize;

  AdblMysqlBindVars* self = ENTC_NEW (AdblMysqlBindVars);
  
  self->max = bindCnt;
  self->pos = 0;
  
  bindSize = sizeof(MYSQL_BIND) * bindCnt;
  if (bindSize > 0)
  {
    // create a new array of binds
    self->binds = ENTC_MALLOC (bindSize);
    
    memset (self->binds, 0, bindSize);
  }
  else
  {
    self->binds = NULL;
  }
    
  return self;
}

//------------------------------------------------------------------------------------------------------

void bindvars_destroy (AdblMysqlBindVars** pself)
{
  AdblMysqlBindVars* self = *pself;
  
  ENTC_FREE (self->binds);
  
  ENTC_DEL (pself, AdblMysqlBindVars);
}

//------------------------------------------------------------------------------------------------------

void bindvars_addS (AdblMysqlBindVars* self, const EcString val)
{
  if (self->pos < self->max)
  {
    // MYSQL_BIND* bind = self->binds + (self->pos * sizeof(MYSQL_BIND));
    MYSQL_BIND* bind = &(self->binds[self->pos]);

    bind->buffer_type = MYSQL_TYPE_STRING;
    bind->buffer = (char*)val;
    bind->buffer_length = strlen (val);
    bind->is_null = 0;
    bind->length = 0;
    bind->error = 0; 
    
    //eclogger_fmt (LL_TRACE, C_MODDESC, "bind val", "bind [%i] value '%s' as string", self->pos, val);
    
    self->pos++;
  }
}

//------------------------------------------------------------------------------------------------------

void bindvars_add (AdblMysqlBindVars* self, EcUdc value)
{
  if (self->pos < self->max)
  {
    //MYSQL_BIND* bind = self->binds + (self->pos * sizeof(MYSQL_BIND));
    MYSQL_BIND* bind = &(self->binds[self->pos]);
    
    switch (ecudc_type(value))
    {
      case ENTC_UDC_STRING:
      {
        const EcString val = ecudc_asString (value);
        
        bind->buffer_type = MYSQL_TYPE_STRING;
        bind->buffer = (char*)val;
        bind->buffer_length = strlen (val);
        bind->is_null = 0;
        bind->length = 0;
        bind->error = 0; 

        //eclogger_fmt (LL_TRACE, C_MODDESC, "bind val", "bind [%i] value '%s' as string", self->pos, val);

        self->pos++;
      }
      break;
      case ENTC_UDC_NUMBER:
      {
        bind->buffer_type = MYSQL_TYPE_LONG;
        
        ecudc_refNumber (value, (void*)&(bind->buffer));
        
        bind->buffer_length = 0;
        bind->is_null = 0;
        bind->length = 0;
        bind->error = 0; 
        bind->is_unsigned = 0;
        
        //eclogger_fmt (LL_TRACE, C_MODDESC, "bind val", "bind [%i] value '%i' as integer", self->pos, *((uint32_t*)bind->buffer));
        
        self->pos++;
      }
      break;
    }
    
  }
}

//------------------------------------------------------------------------------------------------------

void adblmodule_destroy (AdblMysqlConnection* pself)
{
  AdblMysqlConnection self = *pself;
  
  eclog_msg (LL_DEBUG, "MYSQ", "disconnect", "Disconnected from Mysql database" );

  mysql_close (self->conn);
  
  ecmutex_delete (&(self->mutex));
  
  ENTC_DEL (pself, struct AdblMysqlConnection_s);

  // call this after mysql_close
  adblmodule_done ();
}

//------------------------------------------------------------------------------------------------------

void* adblmodule_dbconnect (AdblConnectionProperties* cp)
{
  MYSQL_RES* res;

  AdblMysqlConnection self = ENTC_NEW(struct AdblMysqlConnection_s);
  
  self->mutex = ecmutex_new();
  self->ansi = FALSE;
  
  // call this before mysql_init
  adblmodule_init ();
  
  // init mysql
  self->conn = mysql_init (NULL);
  
  // settings
  mysql_options (self->conn, MYSQL_OPT_COMPRESS, 0);
  
  my_bool reconnect = TRUE;
  mysql_options (self->conn, MYSQL_OPT_RECONNECT, &reconnect);
  
  mysql_options (self->conn, MYSQL_INIT_COMMAND, "SET autocommit=0");

  // connect
  if(!mysql_real_connect (self->conn, cp->host, cp->username, cp->password, cp->schema, cp->port, 0, CLIENT_MULTI_RESULTS))
  {
    eclog_msg (LL_ERROR, "MYSQ", "connect", mysql_error(self->conn) );
    
    adblmodule_destroy (&self);
    
    return NULL;
  }
  
  self->schema = cp->schema;
  
  // find out the ansi variables
  mysql_query (self->conn, "SELECT @@global.sql_mode");
  res = mysql_use_result (self->conn);
  if(res)
  {
    MYSQL_ROW row;
    row = mysql_fetch_row(res);
    if(strstr(row[0], "ANSI_QUOTES" ) != 0) self->ansi = TRUE;
    mysql_free_result(res);
  }
  
  //eclogger_msg (LL_DEBUG, "MYSQ", "connect", "Successful connected to Mysql database" );

  return self;
}

//------------------------------------------------------------------------------------------------------

void adblmodule_dbdisconnect (void* ptr)
{
  AdblMysqlConnection self = ptr;
  
  adblmodule_destroy (&self);  
}

//------------------------------------------------------------------------------------------------------

void adbl_constructListWithTable_Column (EcStream statement, AdblQueryColumn* qc, const char* table, int ansi, EcMap orders, AdblMysqlCursor* cursor, int index)
{
  if( qc->table && qc->ref && qc->value )
  {
    // add subquery
    if(ansi == TRUE)
    {
      ecstream_append_str (statement, "( SELECT " );
      ecstream_append_str (statement, "\"" );
      ecstream_append_str (statement, qc->table );
      ecstream_append_str (statement, "\".\"" );
      ecstream_append_str (statement, qc->value );
      ecstream_append_str (statement, "\" FROM \"" );
      ecstream_append_str (statement, qc->table );
      ecstream_append_str (statement, "\" WHERE \"" );
      ecstream_append_str (statement, qc->table );
      ecstream_append_str (statement, "\".\"" );
      ecstream_append_str (statement, qc->ref );
      ecstream_append_str (statement, "\" = \"" );
      ecstream_append_str (statement, table );
      ecstream_append_str (statement, "\".\"" );
      ecstream_append_str (statement, qc->column );
      ecstream_append_str (statement, "\" )" );
    }
    else
    {
      ecstream_append_str (statement, "( SELECT " );
      ecstream_append_str (statement, qc->table );
      ecstream_append_str (statement, "." );
      ecstream_append_str (statement, qc->value );
      ecstream_append_str (statement, " FROM " );
      ecstream_append_str (statement, qc->table );
      ecstream_append_str (statement, " WHERE " );
      ecstream_append_str (statement, qc->table );
      ecstream_append_str (statement, "." );
      ecstream_append_str (statement, qc->ref );
      ecstream_append_str (statement, " = " );
      ecstream_append_str (statement, table );
      ecstream_append_str (statement, "." );
      ecstream_append_str (statement, qc->column );
      ecstream_append_str (statement, " )" );      
    }
  }
  else
  {
    // normal column
    if(ansi == TRUE)
    {
      ecstream_append_str (statement, "\"" );
      ecstream_append_str (statement, table );
      ecstream_append_str (statement, "\".\"" );
      ecstream_append_str (statement, qc->column );
      ecstream_append_str (statement, "\"" );
    }
    else
    {
      ecstream_append_str (statement, table );
      ecstream_append_str (statement, "." );
      ecstream_append_str (statement, qc->column );    
    }
  }
  
  cursor->bindResult[index].buffer_type = MYSQL_TYPE_STRING;
  cursor->bindResult[index].buffer = 0;
  cursor->bindResult[index].buffer_length = 0;
  cursor->bindResult[index].is_null = &(cursor->is_null[index]);
  cursor->bindResult[index].length = &(cursor->length[index]);
  cursor->bindResult[index].error = &(cursor->error[index]);
  
  if( qc->orderno != 0 )
  {
    long abs_orderno = abs(qc->orderno);
    
    EcBuffer buffer = ecbuf_create (11);
   
    ecstream_append_str (statement, " AS ");
    
    ecbuf_format (buffer, 10, "C%u", abs_orderno );      

    ecstream_append_str (statement, ecbuf_const_str (buffer));
    
    if( qc->orderno > 0 )
    {
      ecmap_insert (orders, (void*)abs_orderno, ecstr_cat2 (ecbuf_const_str (buffer), " ASC"));
    }
    else
    {
      ecmap_insert (orders, (void*)abs_orderno, ecstr_cat2 (ecbuf_const_str (buffer), " DESC"));
    }
    
    ecbuf_destroy (&buffer);
  }
}

//------------------------------------------------------------------------------------------------------

void adbl_constructListWithTable (EcStream statement, EcList columns, const char* table, int ansi, EcMap orders, AdblMysqlCursor* cursor)
{
  EcListCursor c; eclist_cursor_init (columns, &c, LIST_DIR_NEXT);
  
  if (eclist_cursor_next (&c))
  {
    int index = 0;
    
    // first column
    adbl_constructListWithTable_Column( statement, eclist_data(c.node), table, ansi, orders, cursor, index);
    
    index++;
    
    // next columns
    for(; eclist_cursor_next (&c); index++)
    {
      ecstream_append_str (statement, ", " );

      adbl_constructListWithTable_Column( statement, eclist_data(c.node), table, ansi, orders, cursor, index);
    }
  }
  else
  {
    ecstream_append_str (statement, "*" );
  }
}

//------------------------------------------------------------------------------------------------------

void adbl_constructConstraintElement (EcStream statement, AdblConstraintElement* element, int ansi, AdblMysqlBindVars* bv)
{
  if( element->type == QUOMADBL_CONSTRAINT_EQUAL )
  {
    if( element->constraint )
    {
      
    }
    else
    {
      if(ansi == TRUE)
      {
        ecstream_append_str (statement, "\"" );
        ecstream_append_str (statement, ecudc_name(element->data));
        ecstream_append_str (statement, "\" = ?" );
      }
      else
      {
        ecstream_append_str (statement, ecudc_name(element->data) );
        ecstream_append_str (statement, " = ?" );
      }
      
      bindvars_add (bv, element->data);
    }    
  }
}

//------------------------------------------------------------------------------------------------------

void adbl_constructContraintNode (EcStream statement, AdblConstraint* constraint, int ansi, AdblMysqlBindVars* bv)
{
  EcListCursor c;
  eclist_cursor_init (constraint->list, &c, LIST_DIR_NEXT);
  
  if (eclist_cursor_next (&c))
  {
    adbl_constructConstraintElement( statement, eclist_data(c.node), ansi, bv);
    
    while (eclist_cursor_next (&c))
    {
      if( constraint->type == QUOMADBL_CONSTRAINT_AND )
      {
        ecstream_append_str (statement, " AND " );
      }
        
      adbl_constructConstraintElement( statement, eclist_data(c.node), ansi, bv);
    }
  }
}

//------------------------------------------------------------------------------------------------------

void adbl_constructConstraint (EcStream statement, AdblConstraint* constraint, int ansi, AdblMysqlBindVars* bv)
{
  if (eclist_hasContent (constraint->list))
  {
    ecstream_append_str (statement, " WHERE " );
    
    adbl_constructContraintNode (statement, constraint, ansi, bv);
  }  
}

//------------------------------------------------------------------------

static int __STDCALL adblmodule_dbquery_orders_onCmp (const void* a, const void* b)
{
  long ia = (long)a;
  long ib = (long)b;
  
  if (ia > ib)
  {
    return 1;
  }
  else if (ia < ib)
  {
    return -1;
  }
  
  return 0;
}

//------------------------------------------------------------------------

static void __STDCALL adblmodule_dbquery_orders_onDel (void* key, void* val)
{
  EcString h = val; ecstr_delete (&h);
}

//------------------------------------------------------------------------------------------------------

int adblmodule_createStatement (AdblMysqlConnection self, EcStream statement, AdblQuery* query, AdblMysqlBindVars* bv, AdblMysqlCursor* cursor)
{
  int cntBinds = 0;
  
  EcMap orders = ecmap_create (adblmodule_dbquery_orders_onCmp, adblmodule_dbquery_orders_onDel);

  ecstream_append_str (statement, "SELECT " );
  
  adbl_constructListWithTable( statement, query->columns, query->table, self->ansi, orders, cursor);
  
  if (self->ansi == TRUE)
  {
    ecstream_append_str (statement, " FROM \"" );
    ecstream_append_str (statement, self->schema );
    ecstream_append_str (statement, "\".\"" );
    ecstream_append_str (statement, query->table );
    ecstream_append_str (statement, "\"" );
  }
  else
  {
    ecstream_append_str (statement, " FROM " );
    ecstream_append_str (statement, self->schema );
    ecstream_append_str (statement, "." );
    ecstream_append_str (statement, query->table );    
  }
  
  if (query->constraint)
  {
    adbl_constructConstraint (statement, query->constraint, self->ansi, bv);
  }
  
  // apply orders
  {
    EcMapCursor cursor; ecmap_cursor_init (orders, &cursor, LIST_DIR_NEXT);
    
    while (ecmap_cursor_next (&cursor))
    {
      EcMapNode node = cursor.node;
      
      if (cursor.position > 0)
      {
        ecstream_append_str (statement, ", ");
        ecstream_append_str (statement, ecmap_node_value (node));
      }
      else
      {
        ecstream_append_str (statement, " ORDER BY ");
        ecstream_append_str (statement, ecmap_node_value (node));
      }
    }
  }
  
  ecmap_destroy (&orders);
  
  if(query->limit > 0)
  {
    ecstream_append_str (statement, " LIMIT " );
    ecstream_append_u (statement, query->limit );
  }
  
  if(query->offset > 0)
  {
    ecstream_append_str (statement, " OFFSET " );
    ecstream_append_u (statement, query->offset );
  }  
  
  return cntBinds;
}

//------------------------------------------------------------------------------------------------------

AdblMysqlCursor* adblmodule_dbcursor_create (MYSQL_STMT* stmt, int columns)
{
  int bindSize;

  AdblMysqlCursor* self = ENTC_NEW (AdblMysqlCursor);
  
  self->stmt = stmt;
  self->size = columns;
  
  bindSize = sizeof(MYSQL_BIND) * columns;
    
  self->bindResult = (MYSQL_BIND*)ENTC_MALLOC (bindSize);
  memset (self->bindResult, 0x00, bindSize);
  
  self->data = (char*)ENTC_MALLOC (255 * columns);
  
  self->length = (unsigned long*)ENTC_MALLOC (sizeof(unsigned long) * columns);
  self->is_null = (my_bool*)ENTC_MALLOC (sizeof(my_bool) * columns);
  self->error = (my_bool*)ENTC_MALLOC (sizeof(my_bool) * columns);
  
  return self;
}

//------------------------------------------------------------------------------------------------------

int adblmodule_prepared_statement (MYSQL_STMT* stmt, AdblMysqlBindVars* bv, EcStream stream)
{
  // prepare the statement 
  if (mysql_stmt_prepare (stmt, ecstream_get (stream), ecstream_size (stream)) != 0)
  {
    eclog_msg  (LL_ERROR, C_MODDESC, "prepstmt#1", mysql_stmt_error(stmt));
    return FALSE;
  }
  
  //eclogger_fmt (LL_TRACE, C_MODDESC, "bind params", "bind %i parameters", bv->pos);

  // try to bind all constraint values
  if (mysql_stmt_bind_param (stmt, bv->binds) != 0)
  {
    eclog_msg  (LL_ERROR, C_MODDESC, "prepstmt#2", mysql_stmt_error(stmt));
    return FALSE;
  }
  
  return TRUE;
}
  
//------------------------------------------------------------------------------------------------------

void* adblmodule_dbquery_create (AdblMysqlConnection self, AdblQuery* query, MYSQL_STMT* stmt, AdblMysqlBindVars* bv)
{
  int res;
  EcStream statement = ecstream_create ();
  AdblMysqlCursor* cursor = adblmodule_dbcursor_create (stmt, eclist_size (query->columns));
  
  adblmodule_createStatement (self, statement, query, bv, cursor);
  
  //eclogger_msg  (LL_DEBUG, C_MODDESC, "query", ecstream_buffer (statement));

  res = adblmodule_prepared_statement (stmt, bv, statement);

  ecstream_destroy (&statement);
  
  if (!res)
  {
    // clean up
    adblmodule_dbcursor_release (cursor);    
    return NULL;        
  }
  
  // try to bind result
  if (mysql_stmt_bind_result (stmt, cursor->bindResult) != 0)
  {
    //eclogger_msg  (LL_ERROR, C_MODDESC, "query#5", mysql_stmt_error(stmt));
    
    // clean up
    adblmodule_dbcursor_release (cursor);    
    return NULL;        
  }
  
  // execute
  if (mysql_stmt_execute (stmt) != 0)
  {
    eclog_msg  (LL_ERROR, C_MODDESC, "prepstmt#3", mysql_stmt_error(stmt));

    // clean up
    adblmodule_dbcursor_release (cursor);
    return NULL;
  }
  
  if (mysql_stmt_store_result (stmt) != 0)
  {
    eclog_msg  (LL_ERROR, C_MODDESC, "prepstmt#4", mysql_stmt_error(stmt));

    // clean up
    adblmodule_dbcursor_release (cursor);
    return NULL;
  }

  if (mysql_commit (self->conn) != 0)
  {
    eclog_msg  (LL_ERROR, C_MODDESC, "prepstmt#4", mysql_stmt_error(stmt));
    
    // clean up
    adblmodule_dbcursor_release (cursor);
    return NULL;
  }
  
  //db cursor takes the mysql result and destroys it afterwards
  return cursor;
}

//------------------------------------------------------------------------------------------------------

void* adblmodule_dbquery (void* ptr, AdblQuery* query)
{
  MYSQL_STMT* stmt;
  int bindCnt = 0;
  AdblMysqlBindVars* bv;
  void* ret;

  AdblMysqlConnection self = ptr;
    
  ecmutex_lock (self->mutex);
  
  // try to get a prepared statement handle
  stmt = mysql_stmt_init (self->conn);
  if (isNotAssigned (stmt))
  {
    ecmutex_unlock (self->mutex);

    eclog_msg  (LL_ERROR, C_MODDESC, "query#1", mysql_stmt_error(stmt));
    return NULL;
  }
    
  if (query->constraint)
  {
    bindCnt = eclist_size(query->constraint->list);
  }
  
  bv = bindvars_create (bindCnt);
  
  ret = adblmodule_dbquery_create (self, query, stmt, bv);
        
  // clean up
  bindvars_destroy (&bv);
  
  ecmutex_unlock (self->mutex);

  return ret;
}

//------------------------------------------------------------------------------------------------------

int adblmodule_dbprocedure_create (AdblMysqlConnection self, AdblProcedure* proc, MYSQL_STMT* stmt, AdblMysqlBindVars* bv)
{
  int res;
  
  EcStream stream = ecstream_create ();
  
  ecstream_append_str (stream, "CALL ");
  ecstream_append_str (stream, proc->procedure);
  ecstream_append_c (stream, '(');
  
  {
    void* cursor = NULL;
    EcUdc item;
    int pos = 0;
    
    for (item = ecudc_next (proc->values, &cursor); item; item = ecudc_next (proc->values, &cursor), pos++)
    {
      ecstream_append_c (stream, '?');
     
      if (pos > 0)
      {
        ecstream_append_c (stream, ',');
      }
      
      bindvars_add (bv, item);

      //eclogger_fmt (LL_DEBUG, "MYSQ", "bind", "bound item");
    }
  }

  ecstream_append_c (stream, ')');
  
  //eclogger_fmt (LL_DEBUG, "MYSQ", "stmt", ecstream_buffer(stream));
  
  res = adblmodule_prepared_statement (stmt, bv, stream);

  ecstream_destroy (&stream);

  if (!res)
  {
    return FALSE;
  }

  // execute
  if (mysql_stmt_execute (stmt) != 0)
  {
    eclog_msg  (LL_ERROR, C_MODDESC, "prepstmt#3", mysql_stmt_error(stmt));
    
    return FALSE;
  }
  
  //eclogger_fmt (LL_DEBUG, "MYSQ", "stmt", "done");

  return TRUE;
}

//------------------------------------------------------------------------------------------------------

int adblmodule_dbprocedure (void* ptr, AdblProcedure* proc)
{
  int res;
  MYSQL_STMT* stmt;
  AdblMysqlBindVars* bv;
  
  AdblMysqlConnection self = ptr;
  
  ecmutex_lock (self->mutex);

  stmt = mysql_stmt_init (self->conn);
  if (isNotAssigned (stmt))
  {
    ecmutex_unlock (self->mutex);
    
    eclog_msg  (LL_ERROR, C_MODDESC, "proc#1", mysql_stmt_error(stmt));
    return FALSE;
  }
  
  uint32_t size = ecudc_size (proc->values);
  
  //eclogger_fmt (LL_TRACE, "MYSQ", "size", "bind var size %i", size);
  
  bv = bindvars_create (size);

  //eclogger_fmt (LL_TRACE, "MYSQ", "size", "create");

  res = adblmodule_dbprocedure_create (self, proc, stmt, bv);
  
  // clean up
  mysql_stmt_close (stmt);
  
  bindvars_destroy (&bv);
  
  mysql_commit (self->conn);
  
  ecmutex_unlock (self->mutex);
  
  return res;
}

//------------------------------------------------------------------------------------------------------

uint_t adblmodule_dbtable_size (void* ptr, const char* table)
{
  MYSQL_RES* res;

  AdblMysqlConnection self = ptr;

  // use the simple count(*) method to determine the size of the table
  EcStream statement = ecstream_create ();
  
  ecstream_append_str (statement, "SELECT count(*) FROM " );
  
  // schema and table name
  if (self->ansi == TRUE)
  {
    ecstream_append_str (statement, "\"" );
    ecstream_append_str (statement, self->schema );
    ecstream_append_str (statement, "\".\"" );
    ecstream_append_str (statement, table );
    ecstream_append_str (statement, "\"" );
  }
  else
  {
    ecstream_append_str (statement, self->schema );
    ecstream_append_str (statement, "." );
    ecstream_append_str (statement, table );
  }
  
  // log it
  //eclogger_msg (LL_TRACE, "MYSQ", "size", ecstream_buffer (statement));
  
  // execute
  mysql_real_query (self->conn, ecstream_get (statement), ecstream_size (statement));
  
  // release the statement
  ecstream_destroy (&statement);
  
  // get the result
  res = mysql_store_result (self->conn);
  if(res)
  {
    // get the row
    char** row = mysql_fetch_row (res);
    
    // get the value
    if (row)
    {
      return atoi(row[0]);  
    }
    
    // free the result
    mysql_free_result (res);
  }
  else
  {
    if (mysql_errno (self->conn))
    {
      eclog_msg (LL_ERROR, "MYSQ", "size", mysql_error (self->conn));
    }
    else
    {
      eclog_msg (LL_ERROR, "MYSQ", "size", "unknown Mysql error");
    }
  }
  
  return 0;
}

//------------------------------------------------------------------------------------------------------

int adbl_constructAttributesUpdate (EcStream statement, AdblAttributes* attrs, int ansi, AdblMysqlBindVars* bv)
{
  EcMapCursor cursor; ecmap_cursor_init (attrs->columns, &cursor, LIST_DIR_NEXT);
  
  // iterate through all columns
  while (ecmap_cursor_next (&cursor))
  {
    EcMapNode node = cursor.node;
    
    if (cursor.position > 0)
    {
      ecstream_append_str (statement, ", " );
    }

    if(ansi == TRUE)
    {
      ecstream_append_str (statement, "\"" );
      ecstream_append_str (statement, ecmap_node_key (node));
      ecstream_append_str (statement, "\" = ?" );
    }
    else
    {
      ecstream_append_str (statement, ecmap_node_key (node));
      ecstream_append_str (statement, " = ?" );
    }
    
    bindvars_addS (bv, ecmap_node_value(node));
  }
  
  return TRUE;
}

//------------------------------------------------------------------------------------------------------

int adblmodule_dbupdate (void* ptr, AdblUpdate* update, int insert)
{
  int bindCnt;
  AdblMysqlBindVars* bv;
  int res;

  AdblMysqlConnection self = ptr;
  
  // variables
  EcStream statement;
  
  // try to get a prepared statement handle
  MYSQL_STMT* stmt = mysql_stmt_init (self->conn);
  if (isNotAssigned (stmt))
  {
    eclog_msg  (LL_ERROR, C_MODDESC, "update#1", mysql_stmt_error(stmt));
    return 0;
  }
      
  if( !update->constraint )
  {
    return 0;
  }
  
  /* construct the update statement */
  statement = ecstream_create ();
  
  ecstream_append_str (statement, "UPDATE " );
 
  if (self->ansi == TRUE)
  {
    ecstream_append_str (statement, self->schema);
    ecstream_append_str (statement, "\".\"" );
    ecstream_append_str (statement, update->table);
    ecstream_append_str (statement, "\" SET " );
  }
  else
  {
    ecstream_append_str (statement, self->schema);
    ecstream_append_str (statement, "." );
    ecstream_append_str (statement, update->table);
    ecstream_append_str (statement, " SET " );    
  }
  
  bindCnt = ecmap_size (update->attrs->columns) + eclist_size (update->constraint->list);
  
  bv = bindvars_create (bindCnt);

  if (!adbl_constructAttributesUpdate (statement, update->attrs, self->ansi, bv))
  {
    bindvars_destroy (&bv);
    ecstream_destroy (&statement);
    return 0;
  }
  
  adbl_constructConstraint (statement, update->constraint, self->ansi, bv);

  //eclogger_msg (LL_DEBUG, C_MODDESC, "update", ecstream_buffer( statement ) );
  
  res = adblmodule_prepared_statement (stmt, bv, statement);
  
  ecstream_destroy (&statement);

  // execute
  if (mysql_stmt_execute (stmt) != 0)
  {
    mysql_stmt_close (stmt);
    bindvars_destroy (&bv);
    
    eclog_msg  (LL_ERROR, C_MODDESC, "insert failed", mysql_stmt_error(stmt));
    return -1;
  }

  mysql_stmt_close (stmt);
  
  bindvars_destroy (&bv);
  
  // commit
  if (mysql_commit (self->conn) != 0)
  {
    eclog_msg  (LL_ERROR, C_MODDESC, "prepstmt#4", mysql_stmt_error(stmt));
    
    return -1;
  }
  
  if (res)
  {
    return mysql_affected_rows (self->conn);
  }
  else
  {
    return -1;
  }
}

//------------------------------------------------------------------------------------------------------

void adbl_constructAttributesInsert (EcStream statement, AdblMysqlBindVars* bv, AdblAttributes* attrs, int ansi)
{
  EcStream cols;
  EcStream vals;
  
  EcMapCursor cursor; ecmap_cursor_init (attrs->columns, &cursor, LIST_DIR_NEXT);
  
  if( !attrs )
  {
    ecstream_append_str (statement, "VALUES()" );
    return;
  }

  cols = ecstream_create ();
  vals = ecstream_create ();
  
  // iterate through all columns
  while (ecmap_cursor_next (&cursor))
  {
    EcMapNode node = cursor.node;
    
    if (cursor.position > 0)
    {
      ecstream_append_str (cols, ", ");
      ecstream_append_c (vals, ',');
    }
    
    if(ansi == TRUE)
    {
      ecstream_append_c (cols, '"');
      ecstream_append_str (cols, ecmap_node_key (node));
      ecstream_append_c (cols, '"');
      
      ecstream_append_c (vals, '?');
    }
    else
    {
      ecstream_append_str (cols, ecmap_node_key (node));
      
      ecstream_append_c (vals, '?');
    }
    
    bindvars_addS (bv, ecmap_node_value (node));
  }

  ecstream_append_str (statement, "(" );
  ecstream_append_stream (statement, cols);
  ecstream_append_str (statement, ") VALUES (" );
  ecstream_append_stream (statement, vals);
  ecstream_append_str (statement, ")" );
  
  ecstream_destroy (&cols);
  ecstream_destroy (&vals);
}

//------------------------------------------------------------------------------------------------------

int adblmodule_dbinsert (void* ptr, AdblInsert* insert)
{
  EcStream statement;
  int bindCnt;
  AdblMysqlBindVars* bv;
  int affectedRows;

  AdblMysqlConnection self = ptr;
  
  // try to get a prepared statement handle
  MYSQL_STMT* stmt = mysql_stmt_init (self->conn);
  if (isNotAssigned (stmt))
  {
    eclog_msg  (LL_ERROR, C_MODDESC, "delete#1", mysql_stmt_error(stmt));
    return 0;
  }
  
  statement = ecstream_create ();

  if (self->ansi == TRUE)
  {
    ecstream_append_str (statement, "INSERT INTO \"" );
    ecstream_append_str (statement, self->schema );
    ecstream_append_str (statement, "\".\"" );
    ecstream_append_str (statement, insert->table );
    ecstream_append_str (statement, "\" " );
  }
  else
  {
    ecstream_append_str (statement, "INSERT INTO " );
    ecstream_append_str (statement, self->schema );
    ecstream_append_str (statement, "." );
    ecstream_append_str (statement, insert->table );
    ecstream_append_str (statement, " " );    
  }
  
  bindCnt = ecmap_size (insert->attrs->columns);
  
  bv = bindvars_create (bindCnt);
  
  adbl_constructAttributesInsert (statement, bv, insert->attrs, self->ansi);
  
  eclog_msg (LL_DEBUG, C_MODDESC, "insert", ecstream_get (statement) );
  
  adblmodule_prepared_statement (stmt, bv, statement);
  
  // execute
  if (mysql_stmt_execute (stmt) != 0)
  {
    mysql_stmt_close (stmt);
    bindvars_destroy (&bv);

    eclog_msg  (LL_ERROR, C_MODDESC, "insert failed", mysql_stmt_error(stmt));
    return 0;
  }

  mysql_stmt_close (stmt);
  
  ecstream_destroy (&statement);
  
  bindvars_destroy (&bv);
  
  affectedRows = mysql_affected_rows (self->conn);
  
  //eclogger_fmt (LL_TRACE, C_MODDESC, "insert", "affected rows %i", affectedRows);

  // commit
  if (mysql_commit (self->conn) != 0)
  {
    eclog_msg  (LL_ERROR, C_MODDESC, "prepstmt#4", mysql_stmt_error(stmt));
    
    return -1;
  }

  return affectedRows;
}

//------------------------------------------------------------------------------------------------------

int adblmodule_dbdelete (void* ptr, AdblDelete* del)
{
  MYSQL_STMT* stmt;
  EcStream statement;
  int bindCnt;
  AdblMysqlBindVars* bv;
  int res;

  AdblMysqlConnection self = ptr;

  // if we have no constraints return error !
  if (isNotAssigned (del->constraint))
  {
    return 0;
  }
  
  // try to get a prepared statement handle
  stmt = mysql_stmt_init (self->conn);
  if (isNotAssigned (stmt))
  {
    eclog_msg  (LL_ERROR, C_MODDESC, "delete#1", mysql_stmt_error(stmt));
    return 0;
  }
  
  statement = ecstream_create ();

  if (self->ansi == TRUE)
  {
    ecstream_append_str (statement, "DELETE FROM \"" );
    ecstream_append_str (statement, self->schema );
    ecstream_append_str (statement, "\".\"" );
    ecstream_append_str (statement, del->table );
    ecstream_append_str (statement, "\" " );
  }
  else
  {
    ecstream_append_str (statement, "DELETE FROM " );
    ecstream_append_str (statement, self->schema );
    ecstream_append_str (statement, "." );
    ecstream_append_str (statement, del->table );
    ecstream_append_str (statement, " " );    
  }
  
  bindCnt = eclist_size(del->constraint->list);
  
  bv = bindvars_create (bindCnt);

  adbl_constructConstraint (statement, del->constraint, self->ansi, bv);

  //eclogger_msg (LL_DEBUG, C_MODDESC, "delete", ecstream_buffer( statement ) );
    
  res = adblmodule_prepared_statement (stmt, bv, statement);
  
  ecstream_destroy (&statement);

  // execute
  if (mysql_stmt_execute (stmt) != 0)
  {
    eclog_msg  (LL_ERROR, C_MODDESC, "delete failed", mysql_stmt_error(stmt));

    mysql_stmt_close (stmt);
    bindvars_destroy (&bv);
    
    return 0;
  }

  mysql_stmt_close (stmt);
  bindvars_destroy (&bv);
  
  return res;
}

//------------------------------------------------------------------------------------------------------

void adblmodule_dbbegin (void* ptr)
{
  AdblMysqlConnection self = ptr;
  
  mysql_query (self->conn, "START TRANSACTION");

  if (mysql_errno (self->conn))
  {
    eclog_msg (LL_ERROR, "MYSQ", "transaction", mysql_error (self->conn));
  }
  else
  {
    eclog_msg (LL_DEBUG, C_MODDESC, "transaction", "START TRANSACTION");
  }
}

//------------------------------------------------------------------------------------------------------

void adblmodule_dbcommit (void* ptr)
{
  AdblMysqlConnection self = ptr;
  
  mysql_query (self->conn, "COMMIT");

  if (mysql_errno (self->conn))
  {
    eclog_msg (LL_ERROR, "MYSQ", "transaction", mysql_error (self->conn));
  }
  else
  {
    eclog_msg (LL_DEBUG, C_MODDESC, "transaction", "COMMIT");
  }
}

//------------------------------------------------------------------------------------------------------

void adblmodule_dbrollback (void* ptr)
{
  AdblMysqlConnection self = ptr;
  
  mysql_query (self->conn, "ROLLBACK");

  if (mysql_errno (self->conn))
  {
    eclog_msg (LL_ERROR, "MYSQ", "transaction", mysql_error (self->conn));
  }
  else
  {
    eclog_msg (LL_DEBUG, C_MODDESC, "transaction", "ROLLBACK");
  }
}

//------------------------------------------------------------------------------------------------------

int adblmodule_dbcursor_next (void* ptr)
{
  AdblMysqlCursor* self = ptr;
  
  switch (mysql_stmt_fetch (self->stmt))
  {
    case 0:
    {
      return TRUE;
    }
    case 1:
    {
      eclog_msg  (LL_ERROR, C_MODDESC, "fetch", mysql_stmt_error(self->stmt));
      return FALSE;
    }
    case MYSQL_NO_DATA:
    {
      //eclog_msg  (LL_TRACE, C_MODDESC, "fetch", "no data");
      return FALSE;
    }
    case MYSQL_DATA_TRUNCATED:
    {
      //eclog_msg  (LL_WARN, C_MODDESC, "fetch", "data truncated");
      return TRUE;
    }
  }
  
  return TRUE;
}

//------------------------------------------------------------------------------------------------------

const char* adblmodule_dbcursor_data (void* ptr, uint_t column)
{
  AdblMysqlCursor* self = ptr;
  
  if (column < self->size)
  {
    my_bool* isNull = self->is_null + (column * sizeof(my_bool));
    if (*isNull)
    {
      //eclog_fmt (LL_TRACE, C_MODDESC, "fetch data", "got NULL");
      
      return NULL;
    }
    else
    {
      int res;
      unsigned long length = self->length[column];
      
      self->bindResult[column].buffer = realloc(self->bindResult[column].buffer, length + 1);
      self->bindResult[column].buffer_length = length;
      
      res = mysql_stmt_fetch_column (self->stmt, &(self->bindResult[column]), column, 0);
      if (res != 0)
      {
        eclog_fmt (LL_ERROR, C_MODDESC, "fetch data", "got error %i", res);
      }
      
      // set terminator
      ((char*)(self->bindResult[column].buffer))[length] = 0;
      
      //eclog_fmt (LL_TRACE, C_MODDESC, "fetch data", "got '%s' len %i", self->bindResult[column].buffer, length);
      
      return self->bindResult[column].buffer;
    }
  }
  else
  {
    eclog_fmt (LL_WARN, C_MODDESC, "fetch data", "access column %i outside column range %i", column + 1, self->size);
    
    return NULL;
  }
}

//------------------------------------------------------------------------------------------------------

const char* adblmodule_dbcursor_nextdata (void* ptr)
{
  AdblMysqlCursor* self = ptr;
  
  const char* data = adblmodule_dbcursor_data (ptr, self->pos);
  
  self->pos++;
  
  return data;
}

//------------------------------------------------------------------------------------------------------

void adblmodule_dbcursor_release (void* ptr)
{
  int i;

  AdblMysqlCursor* self = ptr;

  mysql_stmt_free_result (self->stmt);
  
  mysql_stmt_close (self->stmt);

  ENTC_FREE (self->data);
  ENTC_FREE (self->is_null);
  ENTC_FREE (self->error);
  ENTC_FREE (self->length);
  
  for (i = 0; i < self->size; i++)
  {
    if (self->bindResult[i].buffer)
    {
      free(self->bindResult[i].buffer);
    }
  }

  // clean up the array
  ENTC_FREE (self->bindResult);
  
  ENTC_DEL (&ptr, AdblMysqlCursor)
  
  //eclogger_fmt (LL_TRACE, C_MODDESC, "fetch data", "cursor released");
}

//------------------------------------------------------------------------------------------------------

void* adblmodule_dbsequence_get (void* ptr, const char* table)
{
  AdblMysqlConnection self = ptr;
  
  // try to get all columns of this table
  MYSQL_RES* res = mysql_list_fields (self->conn, table, 0);
  
  MYSQL_FIELD* field;
  
  EcString column = 0;
  
  if (res == 0)
  {
    if(mysql_errno (self->conn))
    {
      eclog_msg (LL_ERROR, "MYSQ", "sequence", mysql_error (self->conn));
    }
    else
    {
      eclog_msg (LL_ERROR, "MYSQ", "sequence", "unknown Mysql error");
    }

    return NULL;
  }
  
  // iterate through all columns
  for (field = mysql_fetch_field(res); field; field = mysql_fetch_field(res))
  {
    unsigned int flags = field->flags;
    
    if( (flags & PRI_KEY_FLAG) && (flags & AUTO_INCREMENT_FLAG) )
    {
      if (ecstr_valid (column))
      {
        eclog_fmt (LL_ERROR, "MYSQ", "sequence", "Only one auto_increment primary key is allowed for table '%s'", table );
        // clean up
        mysql_free_result(res);
        
        ecstr_delete(&column);
        
        return NULL;
      }
      
      // primary key and auto_increment
      ecstr_replace(&column, field->org_name);
    }
  }
  
  // clean up
  mysql_free_result(res);
  
  if (!ecstr_valid (column))
  {
    eclog_fmt (LL_ERROR, "MYSQ", "sequence", "Please add primary key with auto_increment flag for table '%s'", table );

    return NULL;
  }
  
  {
    AdblMyslSequence* sequence = ENTC_NEW(AdblMyslSequence);

    sequence->table = ecstr_copy (table);
    sequence->mysql = self;
    // ownership transfered to sequence
    sequence->column = column;
  
    return sequence;
  }
}

//------------------------------------------------------------------------------------------------------

void adblmodule_dbsequence_release (void* ptr)
{
  AdblMyslSequence* self = (AdblMyslSequence*)ptr;

  ecstr_delete (&(self->table));
  ecstr_delete (&(self->column));
  
  ENTC_DEL(&self, AdblMyslSequence);
}

//------------------------------------------------------------------------------------------------------

uint_t adblmodule_dbsequence_next (void* ptr)
{
  AdblMyslSequence* sequence = (AdblMyslSequence*)ptr;
  
  // variables
  AdblMysqlConnection mysql;
  
  my_ulonglong unique_id = 0;
  
  // checks
  if( !sequence ) return 0;
  // further settings

  mysql = sequence->mysql;
  
  // add a fake row to the table to increase the autoincrement
  {
    EcStream statement;
    /* first disable foreign key checks */
    mysql_real_query (mysql->conn, "SET FOREIGN_KEY_CHECKS = 0", 26);
    
    if (mysql_errno (mysql->conn))
    {
      if (mysql_errno (mysql->conn))
      {
        eclog_msg (LL_ERROR, "MYSQ", "sequence", mysql_error (mysql->conn));
      }
      else
      {
        eclog_msg (LL_ERROR, "MYSQ", "sequence", "unknown Mysql error");
      }
      
      return 0;
    }
    
    statement = ecstream_create ();
    
    if (mysql->ansi == TRUE)
    {
      ecstream_append_str (statement, "INSERT INTO \"" );
      ecstream_append_str (statement, sequence->table );
      ecstream_append_str (statement, "\" VALUES()" );
    }
    else
    {
      ecstream_append_str (statement, "INSERT INTO " );    
      ecstream_append_str (statement, sequence->table );
      ecstream_append_str (statement, " VALUES()" );
    }
    
    // execute the query
    mysql_real_query (mysql->conn, ecstream_get (statement), ecstream_size (statement));

    // clean up
    ecstream_destroy (&statement);
    
    if (mysql_errno (mysql->conn))
    {
      if (mysql_errno (mysql->conn))
      {
        eclog_msg (LL_ERROR, "MYSQ", "sequence", mysql_error (mysql->conn));
      }
      else
      {
        eclog_msg (LL_ERROR, "MYSQ", "sequence", "unknown Mysql error");
      }
    }
  }
  
  // retrieve the last inserted id
  {
    unique_id = mysql_insert_id (mysql->conn);
    
    if (mysql_errno (mysql->conn))
    {
      if (mysql_errno (mysql->conn))
      {
        eclog_msg (LL_ERROR, "MYSQ", "sequence", mysql_error (mysql->conn));
      }
      else
      {
        eclog_msg (LL_ERROR, "MYSQ", "sequence", "unknown Mysql error");
      }
      
      return 0;
    }
    
    //eclogger_fmt (LL_TRACE, "MYSQ", "sequence", "got new sequence number '%llu'", unique_id );
  }
  
  // delete the fake row
  {
    EcStream statement = ecstream_create ();
    
    if (mysql->ansi == TRUE)
    {
      ecstream_append_str (statement, "DELETE FROM \"" );
      ecstream_append_str (statement, sequence->table );
      ecstream_append_str (statement, "\" WHERE" );
      ecstream_append_str (statement, sequence->column );
      ecstream_append_str (statement, " = " );
      ecstream_append_u ( statement, unique_id );
    }
    else
    {
      ecstream_append_str (statement, "DELETE FROM " );    
      ecstream_append_str (statement, sequence->table );
      ecstream_append_str (statement, " WHERE " );
      ecstream_append_str (statement, sequence->column );
      ecstream_append_str (statement, " = " );
      ecstream_append_u ( statement, unique_id );
    }
    
    // execute the query
    mysql_real_query (mysql->conn, ecstream_get (statement), ecstream_size (statement));

    // clean up
    ecstream_destroy (&statement);

    if (mysql_errno (mysql->conn))
    {
      if (mysql_errno (mysql->conn))
      {
        eclog_msg (LL_ERROR, "MYSQ", "sequence", mysql_error (mysql->conn));
      }
      else
      {
        eclog_msg (LL_ERROR, "MYSQ", "sequence", "unknown Mysql error");
      }
      
      return 0;
    }
  }
  
  // finally enable foreign key checks
  mysql_real_query (mysql->conn, "SET FOREIGN_KEY_CHECKS = 1", 26);
  
  if (mysql_errno (mysql->conn))
  {
    if (mysql_errno (mysql->conn))
    {
      eclog_msg (LL_ERROR, "MYSQ", "sequence", mysql_error (mysql->conn));
    }
    else
    {
      eclog_msg (LL_ERROR, "MYSQ", "sequence", "unknown Mysql error");
    }
    
    return 0;
  }
  
  return unique_id;
}

//------------------------------------------------------------------------------------------------------

EcList adblmodule_dbschema (void* ptr)
{
  return NULL;
}

//------------------------------------------------------------------------------------------------------

AdblTable* adblmodule_dbtable (void* ptr, const EcString tablename)
{
  return NULL;
}

//------------------------------------------------------------------------------------------------------
