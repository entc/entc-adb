#include <system/ecfile.h>
#include <system/ecmutex.h>
#include <types/ecstream.h>
#include <types/eclist.h>
#include <types/ecintmap.h>
#include <utils/ecmessages.h>
#include <utils/eclogger.h>

#include <mysql.h>
#include "adbl.h"

//================================================================================================

#define MODULE "MYSQ"

static const AdblModuleInfo ModuleInfo = { 10000, MODULE, "Mysql" };

#include "adbl_module.inc"

//================================================================================================

struct AdblMysqlConnection
{
  
  MYSQL handle;
  
  char* schema;
  
  int ansi;
  
  EcMutex mutex;
  
};

typedef struct {

  MYSQL_STMT* stmt;
  
  uint_t pos;
  
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
  
  struct AdblMysqlConnection* conn;
  
} AdblMyslSequence;

/*------------------------------------------------------------------------*/

void* adblmodule_dbconnect (AdblConnectionProperties* cp )
{
  struct AdblMysqlConnection* conn = ENTC_NEW(struct AdblMysqlConnection);
  
  conn->mutex = ecmutex_new();
  conn->ansi = FALSE;
  //init the mysql
  MYSQL* mysql = &(conn->handle);

  mysql_init(mysql);
  
  mysql_options(mysql, MYSQL_OPT_RECONNECT, "1");
  
  if(!mysql_real_connect(mysql, cp->host, cp->username, cp->password, cp->schema, cp->port, 0, CLIENT_MULTI_STATEMENTS))
  {
    eclogger_msg (LL_ERROR, "MYSQ", "connect", mysql_error(mysql) );
    
    free(conn);
    
    return 0;
  }
  else
  {
    conn->schema = cp->schema;
    //find out the ansi variables
    mysql_query(mysql, "SELECT @@global.sql_mode");
    MYSQL_RES* res = mysql_use_result(mysql);
    if(res)
    {
      MYSQL_ROW row;
      row = mysql_fetch_row(res);
      if(strstr(row[0], "ANSI_QUOTES" ) != 0) conn->ansi = TRUE;
      mysql_free_result(res);
    }
    mysql_autocommit(mysql, 0);
  }
  
  eclogger_msg (LL_DEBUG, "MYSQ", "connect", "Successful connected to Mysql database" );

  return conn;
}

/*------------------------------------------------------------------------*/

void adblmodule_dbdisconnect (void* ptr)
{
  struct AdblMysqlConnection* conn = ptr;
  
  MYSQL* mysql = &(conn->handle);
  
  mysql_close(mysql);
  
  free(conn);
  
  eclogger_msg (LL_DEBUG, "MYSQ", "disconnect", "Disconnected from Mysql database" );
}

/*------------------------------------------------------------------------*/

void adbl_constructListWithTable_Column (EcStream statement, AdblQueryColumn* qc, const char* table, int ansi, EcIntMap orders)
{
  if( qc->table && qc->ref && qc->value )
  {
    /* add subquery */
    if(ansi == TRUE)
    {
      ecstream_append( statement, "( SELECT " );
      ecstream_append( statement, "\"" );
      ecstream_append( statement, qc->table );
      ecstream_append( statement, "\".\"" );
      ecstream_append( statement, qc->value );
      ecstream_append( statement, "\" FROM \"" );
      ecstream_append( statement, qc->table );
      ecstream_append( statement, "\" WHERE \"" );
      ecstream_append( statement, qc->table );
      ecstream_append( statement, "\".\"" );
      ecstream_append( statement, qc->ref );
      ecstream_append( statement, "\" = \"" );
      ecstream_append( statement, table );
      ecstream_append( statement, "\".\"" );
      ecstream_append( statement, qc->column );
      ecstream_append( statement, "\" )" );
    }
    else
    {
      ecstream_append( statement, "( SELECT " );
      ecstream_append( statement, qc->table );
      ecstream_append( statement, "." );
      ecstream_append( statement, qc->value );
      ecstream_append( statement, " FROM " );
      ecstream_append( statement, qc->table );
      ecstream_append( statement, " WHERE " );
      ecstream_append( statement, qc->table );
      ecstream_append( statement, "." );
      ecstream_append( statement, qc->ref );
      ecstream_append( statement, " = " );
      ecstream_append( statement, table );
      ecstream_append( statement, "." );
      ecstream_append( statement, qc->column );
      ecstream_append( statement, " )" );      
    }
  }
  else
  {
    /* normal column */
    if(ansi == TRUE)
    {
      ecstream_append( statement, "\"" );
      ecstream_append( statement, table );
      ecstream_append( statement, "\".\"" );
      ecstream_append( statement, qc->column );
      ecstream_append( statement, "\"" );
    }
    else
    {
      ecstream_append( statement, table );
      ecstream_append( statement, "." );
      ecstream_append( statement, qc->column );    
    }
  }
  
  if( qc->orderno != 0 )
  {
    uint_t abs_orderno = abs(qc->orderno);
    
    EcBuffer buffer = ecbuf_create (11);
   
    ecstream_append( statement, " AS ");
    
    ecbuf_format (buffer, 10, "C%u", abs_orderno );      

    ecstream_append (statement, ecbuf_const_str (buffer));
    
    if( qc->orderno > 0 )
    {
      ecintmap_append(orders, abs_orderno, ecstr_cat2 (ecbuf_const_str (buffer), " ASC"));
    }
    else
    {
      ecintmap_append(orders, abs_orderno, ecstr_cat2 (ecbuf_const_str (buffer), " DESC"));
    }
    ecbuf_destroy (&buffer);
  }
}

/*------------------------------------------------------------------------*/

void adbl_constructListWithTable (EcStream statement, EcList columns, const char* table, int ansi, EcIntMap orders, AdblMysqlCursor* cursor)
{
  EcListNode node = eclist_first(columns);
  
  if( node != eclist_end(columns) )
  /* more than one entry */
  {
    int index = 0;
    /* first column */
    adbl_constructListWithTable_Column( statement, eclist_data(node), table, ansi, orders );
    
    cursor->bindResult[index].buffer_type = MYSQL_TYPE_STRING;
    cursor->bindResult[index].buffer = &cursor->data[index];
    cursor->bindResult[index].buffer_length = 255;
    cursor->bindResult[index].is_null = &cursor->is_null[index];
    cursor->bindResult[index].length = &cursor->length[index];
    cursor->bindResult[index].error = &cursor->error[index];
    
    /* next columns */
    for(node = eclist_next(node); node != eclist_end(columns); node = eclist_next(node) )
    {
      ecstream_append( statement, ", " );

      adbl_constructListWithTable_Column( statement, eclist_data(node), table, ansi, orders );
    }
  }
  else
  {
    ecstream_append( statement, "*" );
  }
}

/*------------------------------------------------------------------------*/

int adbl_constructConstraintElement (EcStream statement, AdblConstraintElement* element, int ansi, MYSQL_BIND* bind, int bindIndex)
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
        ecstream_append( statement, "\"" );
        ecstream_append( statement, element->column );
        ecstream_append( statement, "\" = ?" );
      }
      else
      {
        ecstream_append( statement, element->column );
        ecstream_append( statement, " = ?" );
      }
      
      bind[bindIndex].buffer_type = MYSQL_TYPE_LONG;
      bind[bindIndex].buffer = element->value;
      bind[bindIndex].buffer_length = strlen (element->value);
      bind[bindIndex].is_null = 0;
      bind[bindIndex].length = 0;
      bind[bindIndex].error = 0;
      
      return bindIndex + 1;
    }    
  }
  return bindIndex;
}

/*------------------------------------------------------------------------*/

int adbl_constructContraintNode (EcStream statement, AdblConstraint* constraint, int ansi, MYSQL_BIND* bind, int bindIndex)
{
  int cntBind = bindIndex;
  
  EcListNode node = eclist_first(constraint->list);
  
  if( node != eclist_end(constraint->list) )
  {
    cntBind = adbl_constructConstraintElement( statement, eclist_data(node), ansi, bind, bindIndex);
    
    node = eclist_next(node);
    
    for(; node != eclist_end(constraint->list); node = eclist_next(node) )
    {
      if( constraint->type == QUOMADBL_CONSTRAINT_AND )
        ecstream_append( statement, " AND " );
      
      cntBind = adbl_constructConstraintElement( statement, eclist_data(node), ansi, bind, bindIndex);
    }
  }
  
  return cntBind;
}

/*------------------------------------------------------------------------*/

int adbl_constructConstraint (EcStream statement, AdblConstraint* constraint, int ansi, MYSQL_BIND* bind)
{
  int cntBinds = 0;
  
  EcListNode node = eclist_first(constraint->list);
  
  if( node != eclist_end(constraint->list) )
  {
    ecstream_append( statement, " WHERE " );
    
    cntBinds = adbl_constructContraintNode (statement, constraint, ansi, bind, cntBinds);
  }  
  
  return cntBinds;
}

/*------------------------------------------------------------------------*/

int adblmodule_createStatement (EcStream statement, AdblQuery* query, struct AdblMysqlConnection* conn, MYSQL_BIND* bindParams, AdblMysqlCursor* cursor)
{
  int cntBinds = 0;
  
  EcIntMap orders = ecintmap_new();

  ecstream_append( statement, "SELECT " );
  
  adbl_constructListWithTable( statement, query->columns, query->table, conn->ansi, orders, cursor);
  
  if(conn->ansi == TRUE)
  {
    ecstream_append( statement, " FROM \"" );
    ecstream_append( statement, conn->schema );
    ecstream_append( statement, "\".\"" );
    ecstream_append( statement, query->table );
    ecstream_append( statement, "\"" );
  }
  else
  {
    ecstream_append( statement, " FROM " );
    ecstream_append( statement, conn->schema );
    ecstream_append( statement, "." );
    ecstream_append( statement, query->table );    
  }
  
  if(query->constraint)
  {
    cntBinds = adbl_constructConstraint (statement, query->constraint, conn->ansi, bindParams);
  }
  
  /* apply the order */
  ecintmap_orderAll(orders);
  
  if( ecintmap_first(orders) != ecintmap_end(orders) )
  {
    EcIntMapNode orders_node = ecintmap_first(orders);
    
    char* alias = ecintmap_data(orders_node);
    
    ecstream_append( statement, " ORDER BY " );
    
    ecstream_append( statement, alias );
    
    ecstr_delete(&alias);
    
    orders_node = ecintmap_next(orders_node);
    
    for (; orders_node != ecintmap_end(orders); orders_node = ecintmap_next(orders_node))
    {
      alias = ecintmap_data(orders_node);
      
      ecstream_append( statement, ", " );
      
      ecstream_append( statement, alias );      
      
      ecstr_delete(&alias);
    }    
  }
  
  ecintmap_delete(&orders);
  
  if(query->limit > 0)
  {
    ecstream_append( statement, " LIMIT " );
    ecstream_appendu( statement, query->limit );
  }
  if(query->offset > 0)
  {
    ecstream_append( statement, " OFFSET " );
    ecstream_appendu( statement, query->offset );
  }  
  
  return cntBinds;
}

//--------------------------------------------------------------------------

void* adblmodule_dbquery (void* ptr, AdblQuery* query)
{
  struct AdblMysqlConnection* conn = ptr;
    
  // try to get a prepared statement handle
  MYSQL_STMT* stmt = mysql_stmt_init (&(conn->handle));
  if (isNotAssigned (stmt))
  {
    eclogger_msg  (LL_ERROR, "__01", "query", mysql_stmt_error(stmt));
    return NULL;
  }
  
  MYSQL_BIND* bindParams = (MYSQL_BIND*)ENTC_MALLOC (sizeof(MYSQL_BIND) * 10);
  memset (bindParams, 0x00, sizeof(bindParams));
  
  AdblMysqlCursor* cursor = ENTC_NEW (AdblMysqlCursor);
  
  cursor->stmt = stmt;
  
  cursor->bindResult = (MYSQL_BIND*)ENTC_MALLOC (sizeof(MYSQL_BIND) * 10);
  memset (cursor->bindResult, 0x00, sizeof(cursor->bindResult));

  cursor->data = (char*)ENTC_MALLOC (255 * 10);

  cursor->length = (unsigned long*)ENTC_MALLOC (sizeof(unsigned long) * 10);
  cursor->is_null = (my_bool*)ENTC_MALLOC (sizeof(my_bool) * 10);
  cursor->error = (my_bool*)ENTC_MALLOC (sizeof(my_bool) * 10);
  
  EcStream statement = ecstream_new();
  adblmodule_createStatement (statement, query, conn, bindParams, cursor);

  // prepare the statement 
  if (mysql_stmt_prepare (stmt, ecstream_buffer (statement), ecstream_size(statement)))
  {
    eclogger_msg  (LL_ERROR, "__01", "query", mysql_stmt_error(stmt));

    // clean up
    mysql_stmt_close (stmt);
    return NULL;
  }
  
  if(query->constraint)
  {
    // try to bind all contraint values
    my_bool res = mysql_stmt_bind_param (stmt, bindParams);
    if (!res)
    {
      eclogger_msg (LL_ERROR, "MSQL", "query", mysql_stmt_error(stmt));
      
      // clean up
      mysql_stmt_close (stmt);
      return NULL;    
    }    
  }
  
  // execute
  int resExec = mysql_stmt_execute (stmt);
  if (resExec != 0)
  {
    eclogger_msg  (LL_ERROR, "__03", "query", mysql_stmt_error(stmt));
    
    // clean up
    mysql_stmt_close (stmt);
    return NULL;    
  }
  
  // try to bind result
  if (mysql_stmt_bind_result(stmt, cursor->bindResult))
  {
    eclogger_msg  (LL_ERROR, "__04", "query", mysql_stmt_error(stmt));
    
    // clean up
    mysql_stmt_close (stmt);
    return NULL;        
  }
  
  //db cursor takes the mysql result and destroys it afterwards    
  ecstream_delete (&statement);
    
  return cursor;
}

/*------------------------------------------------------------------------*/

uint_t adblmodule_dbtable_size (void* ptr, const char* table)
{
  struct AdblMysqlConnection* conn = ptr;
  /* use the simple count(*) method to determine the size of the table */  
  EcStream statement = ecstream_new();
  
  ecstream_append( statement, "SELECT count(*) FROM " );
  /* schema and table name */  
  if(conn->ansi == TRUE)
  {
    ecstream_append( statement, "\"" );
    ecstream_append( statement, conn->schema );
    ecstream_append( statement, "\".\"" );
    ecstream_append( statement, table );
    ecstream_append( statement, "\"" );
  }
  else
  {
    ecstream_append( statement, conn->schema );
    ecstream_append( statement, "." );
    ecstream_append( statement, table );
  }
  /* log it */
  eclogger_msg (LL_TRACE, "MYSQ", "size", ecstream_buffer( statement ) );
  /* execute */
  mysql_real_query( &(conn->handle), ecstream_buffer( statement ), ecstream_size( statement ) );
  /* release the statement */  
  ecstream_delete (&statement);
  /* get the result */
  MYSQL_RES* res = mysql_store_result(&(conn->handle));
  if(res)
  {
    /* get the row */
    char** row = mysql_fetch_row( res );
    /* get the value */
    if(row)
    {
      return atoi(row[0]);  
    }
    /* free the result */
    mysql_free_result( res );
  }
  else
  {
    if(mysql_errno( &(conn->handle) ))
      eclogger_msg (LL_ERROR, "MYSQ", "size", mysql_error( &(conn->handle) ) );
    else
      eclogger_msg (LL_ERROR, "MYSQ", "size", "unknown Mysql error" );    
  }
  return 0;
}

/*------------------------------------------------------------------------*/

int adbl_constructAttributesUpdate (EcStream statement, AdblAttributes* attrs, int ansi )
{  
  EcMapCharNode node = ecmapchar_first(attrs->columns);
  
  if( node != ecmapchar_end(attrs->columns) )
  {
    if(ansi == TRUE)
    {
      ecstream_append( statement, "\"" );
      ecstream_append( statement, ecmapchar_key(node) );
      ecstream_append( statement, "\" = \'" );
      ecstream_append( statement, ecmapchar_data(node) );
      ecstream_append( statement, "\'" );
    }
    else
    {
      ecstream_append( statement, ecmapchar_key(node) );
      ecstream_append( statement, " = \"" );
      ecstream_append( statement, ecmapchar_data(node) );
      ecstream_append( statement, "\"" );      
    }
    
    node = ecmapchar_next(node);
    
    for(; node != ecmapchar_end(attrs->columns); node = ecmapchar_next(node) )
    {
      if(ansi == TRUE)
      {
        ecstream_append( statement, ", \"" );
        ecstream_append( statement, ecmapchar_key(node) );
        ecstream_append( statement, "\" = \'" );
        ecstream_append( statement, ecmapchar_data(node) );
        ecstream_append( statement, "\'" );
      }
      else
      {
        ecstream_append( statement, ", " );
        ecstream_append( statement, ecmapchar_key(node) );
        ecstream_append( statement, " = \"" );
        ecstream_append( statement, ecmapchar_data(node) );
        ecstream_append( statement, "\"" );      
      }      
    }
    return TRUE;
  }
  return FALSE;
}

/*------------------------------------------------------------------------*/

int adblmodule_dbupdate (void* ptr, AdblUpdate* update, int insert)
{
  /* cast */
  struct AdblMysqlConnection* conn = ptr;
  /* variables */
  EcStream statement;
  int rows = 0;
    
  if( !update->constraint )
  {
    return 0;
  }
  /* construct the update statement */
  statement = ecstream_new();
  
  ecstream_append( statement, "UPDATE " );
 
  if(conn->ansi == TRUE)
  {
    ecstream_append( statement, conn->schema );
    ecstream_append( statement, "\".\"" );
    ecstream_append( statement, update->table );
    ecstream_append( statement, "\" SET " );
  }
  else
  {
    ecstream_append( statement, conn->schema );
    ecstream_append( statement, "." );
    ecstream_append( statement, update->table );
    ecstream_append( statement, " SET " );    
  }
  
  if( !adbl_constructAttributesUpdate(statement, update->attrs, conn->ansi ) )
  {
    ecstream_delete (&statement);
    
    return 0;
  }
  
  adbl_constructConstraint( statement, update->constraint, conn->ansi, NULL );
    
  eclogger_msg (LL_TRACE, "MYSQ", "update", ecstream_buffer( statement ) );
    
  //MUTEX_LOCK(_mutex);
    
    
  mysql_real_query( &(conn->handle), ecstream_buffer( statement ), ecstream_size( statement ) );
  
  if(mysql_errno( &(conn->handle) ))
  {
    if(mysql_errno( &(conn->handle) ))
    {
      eclogger_msg (LL_ERROR, "MYSQ", "query", mysql_error( &(conn->handle) ) );
    }
    else
    {
      eclogger_msg (LL_ERROR, "MYSQ", "query", "unknown Mysql error" );
    }
    /* set rows to an error */
    rows = -1;
  }
  else
  {
    rows = mysql_affected_rows( &(conn->handle) );
  }

  ecstream_delete (&statement);
  
  return rows;
}

/*------------------------------------------------------------------------*/

void adbl_constructAttributesInsert (EcStream statement, AdblAttributes* attrs, int ansi )
{
  if( !attrs )
  {
    ecstream_append( statement, "VALUES()" );
    return;
  }
  
  EcMapCharNode node = ecmapchar_first(attrs->columns);
  
  if( node != ecmapchar_end(attrs->columns) )
  {
    EcStream cols = ecstream_new();
    EcStream values = ecstream_new();

    if(ansi == TRUE)
    {
      ecstream_append( cols, "\"" );
      ecstream_append( cols, ecmapchar_key(node) );
      ecstream_append( cols, "\"" );

      ecstream_append( values, "\'" );
      ecstream_append( values, ecmapchar_data(node) );
      ecstream_append( values, "\'" );
    }
    else
    {
      ecstream_append( cols, ecmapchar_key(node) );

      ecstream_append( values, "\"" );
      ecstream_append( values, ecmapchar_data(node) );
      ecstream_append( values, "\"" );      
    }
    
    node = ecmapchar_next(node);
    
    for(; node != ecmapchar_end(attrs->columns); node = ecmapchar_next(node) )
    {
      if(ansi == TRUE)
      {
        ecstream_append( cols, ", \"" );
        ecstream_append( cols, ecmapchar_key(node) );
        ecstream_append( cols, "\"" );
        
        ecstream_append( values, ", \'" );
        ecstream_append( values, ecmapchar_data(node) );
        ecstream_append( values, "\'" );
      }
      else
      {
        ecstream_append( cols, ", " );
        ecstream_append( cols, ecmapchar_key(node) );
        
        ecstream_append( values, ", \"" );
        ecstream_append( values, ecmapchar_data(node) );
        ecstream_append( values, "\"" );      
      }      
    }
    ecstream_append( statement, "(" );
    ecstream_append( statement, ecstream_buffer( cols ) );
    ecstream_append( statement, ") VALUES (" );
    ecstream_append( statement, ecstream_buffer( values ) );
    ecstream_append( statement, ")" );
    
    ecstream_delete (&cols);
    ecstream_delete (&values);
  }
  else
  {
    ecstream_append( statement, "VALUES()" );
  }
}

/*------------------------------------------------------------------------*/

int adblmodule_dbinsert (void* ptr, AdblInsert* insert)
{
  struct AdblMysqlConnection* conn = ptr;
  
  EcStream statement = ecstream_new();

  if(conn->ansi == TRUE)
  {
    ecstream_append( statement, "INSERT INTO \"" );
    ecstream_append( statement, conn->schema );
    ecstream_append( statement, "\".\"" );
    ecstream_append( statement, insert->table );
    ecstream_append( statement, "\" " );
  }
  else
  {
    ecstream_append( statement, "INSERT INTO " );
    ecstream_append( statement, conn->schema );
    ecstream_append( statement, "." );
    ecstream_append( statement, insert->table );
    ecstream_append( statement, " " );    
  }
  
  adbl_constructAttributesInsert(statement, insert->attrs, conn->ansi );
  
  eclogger_msg (LL_TRACE, "MYSQ", "insert", ecstream_buffer( statement ) );
  
  //MUTEX_LOCK(_mutex);
  
  mysql_real_query( &(conn->handle), ecstream_buffer( statement ), ecstream_size( statement ) );
  
  ecstream_delete (&statement);
  
  if(mysql_errno( &(conn->handle) ))
  {
    if(mysql_errno( &(conn->handle) ))
      eclogger_msg (LL_ERROR, "MYSQ", "insert", mysql_error( &(conn->handle) ) );
    else
      eclogger_msg (LL_ERROR, "MYSQ", "insert", "unknown Mysql error" );
    
    return 0;
  }
    
  return mysql_affected_rows( &(conn->handle) );
}

/*------------------------------------------------------------------------*/

int adblmodule_dbdelete (void* ptr, AdblDelete* del)
{
  /* if we have no constraints return error ! */
  if( !del->constraint )
    return 0;
  
  struct AdblMysqlConnection* conn = ptr;
  
  EcStream statement = ecstream_new();

  if(conn->ansi == TRUE)
  {
    ecstream_append( statement, "DELETE FROM \"" );
    ecstream_append( statement, conn->schema );
    ecstream_append( statement, "\".\"" );
    ecstream_append( statement, del->table );
    ecstream_append( statement, "\" " );
  }
  else
  {
    ecstream_append( statement, "DELETE FROM " );
    ecstream_append( statement, conn->schema );
    ecstream_append( statement, "." );
    ecstream_append( statement, del->table );
    ecstream_append( statement, " " );    
  }
  
  adbl_constructConstraint( statement, del->constraint, conn->ansi, NULL );

  eclogger_msg (LL_TRACE, "MYSQ", "delete", ecstream_buffer( statement ) );
    
  mysql_real_query( &(conn->handle), ecstream_buffer( statement ), ecstream_size( statement ) );
  
  ecstream_delete (&statement);
  
  if(mysql_errno( &(conn->handle) ))
  {
    if(mysql_errno( &(conn->handle) ))
      eclogger_msg (LL_ERROR, "MYSQ", "delete", mysql_error( &(conn->handle) ) );
    else
      eclogger_msg (LL_ERROR, "MYSQ", "delete", "unknown Mysql error" );
    
    return 0;
  }  
  return TRUE;
}

/*------------------------------------------------------------------------*/

void adblmodule_dbbegin( void* ptr )
{
  struct AdblMysqlConnection* conn = ptr;
  
  mysql_query(&(conn->handle), "START TRANSACTION");
}

/*------------------------------------------------------------------------*/

void adblmodule_dbcommit( void* ptr )
{
  struct AdblMysqlConnection* conn = ptr;
  
  mysql_query(&(conn->handle), "COMMIT");
}

/*------------------------------------------------------------------------*/

void adblmodule_dbrollback( void* ptr )
{
  struct AdblMysqlConnection* conn = ptr;
  
  mysql_query(&(conn->handle), "ROLLBACK");
}

/*------------------------------------------------------------------------*/

int adblmodule_dbcursor_next( void* ptr )
{
  AdblMysqlCursor* self = ptr;
  
  int res = mysql_stmt_fetch (self->stmt);
    
  switch (res)
  {
    case 0:
    {
      return TRUE;
    }
    break;
    case MYSQL_NO_DATA:
    {
      return FALSE;
    }
    break;
    case 1:
    {
      //eclogger_msg  (LL_ERROR, "MYSQ", mysql_stmt_error(self->stmt));
      return FALSE;
    }
    break;
  }
  return FALSE;
}

/*------------------------------------------------------------------------*/

const char* adblmodule_dbcursor_data (void* ptr, uint_t column)
{
  AdblMysqlCursor* self = ptr;

  return self->data + column * 255;
}

/*------------------------------------------------------------------------*/

const char* adblmodule_dbcursor_nextdata (void* ptr)
{
  AdblMysqlCursor* self = ptr;
  
  const char* res = self->data + self->pos * 255;
  self->pos++;
  
  return res;
}

/*------------------------------------------------------------------------*/

void adblmodule_dbcursor_release (void* ptr)
{
  AdblMysqlCursor* self = ptr;

  mysql_stmt_close (self->stmt);

  ENTC_FREE (self->data);
  ENTC_FREE (self->is_null);
  ENTC_FREE (self->error);
  ENTC_FREE (self->length);
  
  ENTC_DEL (&ptr, AdblMysqlCursor)
}

/*------------------------------------------------------------------------*/

void* adblmodule_dbsequence_get (void* ptr, const char* table)
{
  struct AdblMysqlConnection* conn = ptr;
  
  /* try to get all columns of this table */
  MYSQL_RES* res = mysql_list_fields(&(conn->handle), table, 0);
  
  MYSQL_FIELD* field;
  
  EcString column = 0;
  
  if(res == 0)
  {
    if(mysql_errno( &(conn->handle) ))
    {
      eclogger_msg (LL_ERROR, "MYSQ", "sequence", mysql_error( &(conn->handle) ) );
    }
    else
    {
      eclogger_msg (LL_ERROR, "MYSQ", "sequence", "unknown Mysql error" );
    }

    return 0;
  }
  /* iterate through all columns */
  for( field = mysql_fetch_field(res); field; field = mysql_fetch_field(res))
  {
    unsigned int flags = field->flags;
    
    if( (flags & PRI_KEY_FLAG) && (flags & AUTO_INCREMENT_FLAG) )
    {
      if (ecstr_valid (column))
      {
        eclogger_fmt (LL_ERROR, "MYSQ", "sequence", "Only one auto_increment primary key is allowed for table '%s'", table );
        /* clean up */
        mysql_free_result(res);
        
        ecstr_delete(&column);
        
        return 0;
      }
      /* primary key and auto_increment */
      ecstr_replace(&column, field->org_name);
    }
  }
  /* clean up */
  mysql_free_result(res);
  
  if (!ecstr_valid (column))
  {
    eclogger_fmt (LL_ERROR, "MYSQ", "sequence", "Please add primary key with auto_increment flag for table '%s'", table );

    return 0;
  }
  
  AdblMyslSequence* sequence = ENTC_NEW(AdblMyslSequence);

  sequence->table = ecstr_copy (table);
  sequence->conn = conn;
  // ownership transfered to sequence
  sequence->column = column;
  
  return sequence;
}

/*------------------------------------------------------------------------*/

void adblmodule_dbsequence_release (void* ptr)
{
  /* casts */
  AdblMyslSequence* self = (AdblMyslSequence*)ptr;

  ecstr_delete (&(self->table));
  ecstr_delete (&(self->column));
  
  ENTC_DEL(&self, AdblMyslSequence);
}

/*------------------------------------------------------------------------*/

uint_t adblmodule_dbsequence_next (void* ptr)
{
  // casts
  AdblMyslSequence* sequence = (AdblMyslSequence*)ptr;
  // variables
  struct AdblMysqlConnection* conn;  
  my_ulonglong unique_id = 0;
  // checks
  if( !sequence ) return 0;
  // further settings
  conn = sequence->conn;
  /* add a fake row to the table to increase the autoincrement */
  {
    EcStream statement;
    /* first disable foreign key checks */
    mysql_real_query( &(conn->handle), "SET FOREIGN_KEY_CHECKS = 0", 26 );
    
    if(mysql_errno( &(conn->handle) ))
    {
      if(mysql_errno( &(conn->handle) ))
        eclogger_msg (LL_ERROR, "MYSQ", "sequence", mysql_error( &(conn->handle) ) );
      else
        eclogger_msg (LL_ERROR, "MYSQ", "sequence", "unknown Mysql error" );
            
      return 0;
    }
    
    statement = ecstream_new();
    
    if(conn->ansi == TRUE)
    {
      ecstream_append( statement, "INSERT INTO \"" );
      ecstream_append( statement, sequence->table );
      ecstream_append( statement, "\" VALUES()" );
    }
    else
    {
      ecstream_append( statement, "INSERT INTO " );    
      ecstream_append( statement, sequence->table );
      ecstream_append( statement, " VALUES()" );
    }
    
    /* execute the query */
    mysql_real_query( &(conn->handle), ecstream_buffer( statement ), ecstream_size( statement ) );
    /* clean up */    
    ecstream_delete (&statement);
    
    if(mysql_errno( &(conn->handle) ))
    {
      if(mysql_errno( &(conn->handle) ))
        eclogger_msg (LL_ERROR, "MYSQ", "sequence", mysql_error( &(conn->handle) ) );
      else
        eclogger_msg (LL_ERROR, "MYSQ", "sequence", "unknown Mysql error" );      
    }    
  }
  /* retrieve the last inserted id */
  {
    unique_id = mysql_insert_id(&(conn->handle));
    
    if(mysql_errno( &(conn->handle) ))
    {
      if(mysql_errno( &(conn->handle) ))
        eclogger_msg (LL_ERROR, "MYSQ", "sequence", mysql_error( &(conn->handle) ) );
      else
        eclogger_msg (LL_ERROR, "MYSQ", "sequence", "unknown Mysql error" );
      
      return 0;
    }
    
    eclogger_fmt (LL_TRACE, "MYSQ", "sequence", "got new sequence number '%llu'", unique_id );
  }
  /* delete the fake row */
  {
    EcStream statement = ecstream_new();
    
    if(conn->ansi == TRUE)
    {
      ecstream_append( statement, "DELETE FROM \"" );
      ecstream_append( statement, sequence->table );
      ecstream_append( statement, "\" WHERE" );
      ecstream_append( statement, sequence->column );
      ecstream_append( statement, " = " );
      ecstream_appendu( statement, unique_id );
    }
    else
    {
      ecstream_append( statement, "DELETE FROM " );    
      ecstream_append( statement, sequence->table );
      ecstream_append( statement, " WHERE " );
      ecstream_append( statement, sequence->column );
      ecstream_append( statement, " = " );
      ecstream_appendu( statement, unique_id );
    }
    /* execute the query */
    mysql_real_query( &(conn->handle), ecstream_buffer( statement ), ecstream_size( statement ) );

    /* clean up */    
    ecstream_delete (&statement);

    if(mysql_errno( &(conn->handle) ))
    {
      if(mysql_errno( &(conn->handle) ))
        eclogger_msg (LL_ERROR, "MYSQ", "sequence", mysql_error( &(conn->handle) ) );
      else
        eclogger_msg (LL_ERROR, "MYSQ", "sequence", "unknown Mysql error" );
      
      return 0;
    }
  }
  
  /* finally enable foreign key checks */
  mysql_real_query( &(conn->handle), "SET FOREIGN_KEY_CHECKS = 1", 26 );
  
  if(mysql_errno( &(conn->handle) ))
  {
    if(mysql_errno( &(conn->handle) ))
      eclogger_msg (LL_ERROR, "MYSQ", "sequence", mysql_error( &(conn->handle) ) );
    else
      eclogger_msg (LL_ERROR, "MYSQ", "sequence", "unknown Mysql error" );
    
    return 0;
  }
  
  
  return unique_id;
}

//----------------------------------------------------------------------------------------

EcList adblmodule_dbschema (void* ptr)
{
  return NULL;
}

//----------------------------------------------------------------------------------------

AdblTable* adblmodule_dbtable (void* ptr, const EcString tablename)
{
  return NULL;
}

//----------------------------------------------------------------------------------------
