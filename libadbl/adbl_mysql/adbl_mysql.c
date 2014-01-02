#include <system/ecfile.h>
#include <system/ecmutex.h>
#include <types/ecstream.h>
#include <types/eclist.h>
#include <types/ecintmap.h>

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

struct AdblMysqlCursor
{

  MYSQL_RES* res;
  
  MYSQL_ROW row;
  
  uint_t pos;
  
  EcMutex mutex;
  
};

typedef struct 
{
  
  char* table;
  
  char* column;
  
  struct AdblMysqlConnection* conn;
  
} AdblMyslSequence;

/*------------------------------------------------------------------------*/

void* adblmodule_dbconnect(AdblConnectionProperties* cp, EcLogger logger )
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
    eclogger_log(logger, LL_ERROR, "MYSQ", mysql_error(mysql) );
    
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
  
  eclogger_log(logger, LL_DEBUG, "MYSQ", "Successful connected to Mysql database" );

  return conn;
}

/*------------------------------------------------------------------------*/

void adblmodule_dbdisconnect( void* ptr, EcLogger logger )
{
  struct AdblMysqlConnection* conn = ptr;
  
  MYSQL* mysql = &(conn->handle);
  
  mysql_close(mysql);
  
  free(conn);
  
  eclogger_log(logger, LL_DEBUG, "MYSQ", "Disconnected from Mysql database" );
}

/*------------------------------------------------------------------------*/

void adbl_constructListWithTable_Column(EcStream statement, AdblQueryColumn* qc, const char* table, int ansi, EcIntMap orders )
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
    
    EcBuffer buffer = ecstr_buffer(11);
   
    ecstream_append( statement, " AS ");
    
    ecstr_format(buffer, 10, "C%u", abs_orderno );      

    ecstream_append (statement, ecstr_get(buffer));
    
    if( qc->orderno > 0 )
    {
      ecintmap_append(orders, abs_orderno, ecstr_cat2 (ecstr_get(buffer), " ASC"));
    }
    else
    {
      ecintmap_append(orders, abs_orderno, ecstr_cat2 (ecstr_get(buffer), " DESC"));
    }
    ecstr_release(&buffer);
  }
}

/*------------------------------------------------------------------------*/

void adbl_constructListWithTable (EcStream statement, EcList columns, const char* table, int ansi, EcIntMap orders )
{
  EcListNode node = eclist_first(columns);
  
  if( node != eclist_end(columns) )
  /* more than one entry */
  {
    /* first column */
    adbl_constructListWithTable_Column( statement, eclist_data(node), table, ansi, orders );
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

void adbl_constructConstraintElement (EcStream statement, AdblConstraintElement* element, int ansi )
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
        ecstream_append( statement, "\" = \'" );
        ecstream_append( statement, element->value );
        ecstream_append( statement, "\'" );
      }
      else
      {
        ecstream_append( statement, element->column );
        ecstream_append( statement, " = \'" );
        ecstream_append( statement, element->value );
        ecstream_append( statement, "\'" );
      }      
    }    
  }
}

/*------------------------------------------------------------------------*/

void adbl_constructContraintNode( EcStream statement, AdblConstraint* constraint, int ansi )
{
  EcListNode node = eclist_first(constraint->list);
  
  if( node != eclist_end(constraint->list) )
  {
    adbl_constructConstraintElement( statement, eclist_data(node), ansi );
    
    node = eclist_next(node);
    
    for(; node != eclist_end(constraint->list); node = eclist_next(node) )
    {
      if( constraint->type == QUOMADBL_CONSTRAINT_AND )
        ecstream_append( statement, " AND " );
      
      adbl_constructConstraintElement( statement, eclist_data(node), ansi );
    }
  }
}

/*------------------------------------------------------------------------*/

void adbl_constructConstraint (EcStream statement, AdblConstraint* constraint, int ansi )
{
  EcListNode node = eclist_first(constraint->list);
  
  if( node != eclist_end(constraint->list) )
  {
    ecstream_append( statement, " WHERE " );
    
    adbl_constructContraintNode( statement, constraint, ansi );
  }  
}

/*------------------------------------------------------------------------*/

void* adblmodule_dbquery( void* ptr, AdblQuery* query, EcLogger logger )
{
  struct AdblMysqlConnection* conn = ptr;
  
  EcStream statement = ecstream_new();
  
  struct AdblMysqlCursor* cursor = 0;

  EcIntMap orders = ecintmap_new();

  ecstream_append( statement, "SELECT " );
  
  adbl_constructListWithTable( statement, query->columns, query->table, conn->ansi, orders );
  
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
    adbl_constructConstraint( statement, query->constraint, conn->ansi );
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
  
  eclogger_log(logger, LL_TRACE, "MYSQ", ecstream_buffer( statement ) );

  ecmutex_lock(conn->mutex);
  
  mysql_real_query( &(conn->handle), ecstream_buffer( statement ), ecstream_size( statement ) );
  
  ecstream_delete (&statement);
  
  MYSQL_RES* res = mysql_store_result(&(conn->handle));
  if(res)
  {
    //db cursor takes the mysql result and destroys it afterwards
    cursor = ENTC_NEW(struct AdblMysqlCursor);
    
    cursor->res = res;
    cursor->pos = 0;
    cursor->row = 0;
    cursor->mutex = conn->mutex;
  }
  else
  {
    if(mysql_errno( &(conn->handle) ))
      eclogger_log(logger, LL_ERROR, "MYSQ", mysql_error( &(conn->handle) ) );
    else
      eclogger_log(logger, LL_ERROR, "MYSQ", "unknown Mysql error" );
  }
  
  ecmutex_unlock(conn->mutex);
  
  return cursor;
}

/*------------------------------------------------------------------------*/

uint_t adblmodule_dbtable_size (void* ptr, const char* table, EcLogger logger)
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
  eclogger_log(logger, LL_TRACE, "MYSQ", ecstream_buffer( statement ) );
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
      eclogger_log(logger, LL_ERROR, "MYSQ", mysql_error( &(conn->handle) ) );
    else
      eclogger_log(logger, LL_ERROR, "MYSQ", "unknown Mysql error" );    
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

int adblmodule_dbupdate (void* ptr, AdblUpdate* update, EcLogger logger)
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
  
  adbl_constructConstraint( statement, update->constraint, conn->ansi );
    
  eclogger_log(logger, LL_TRACE, "MYSQ", ecstream_buffer( statement ) );
    
  //MUTEX_LOCK(_mutex);
    
    
  mysql_real_query( &(conn->handle), ecstream_buffer( statement ), ecstream_size( statement ) );
  
  if(mysql_errno( &(conn->handle) ))
  {
    if(mysql_errno( &(conn->handle) ))
    {
      eclogger_log(logger, LL_ERROR, "MYSQ", mysql_error( &(conn->handle) ) );
    }
    else
    {
      eclogger_log(logger, LL_ERROR, "MYSQ", "unknown Mysql error" );
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

int adblmodule_dbinsert (void* ptr, AdblInsert* insert, EcLogger logger)
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
  
  eclogger_log(logger, LL_TRACE, "MYSQ", ecstream_buffer( statement ) );
  
  //MUTEX_LOCK(_mutex);
  
  mysql_real_query( &(conn->handle), ecstream_buffer( statement ), ecstream_size( statement ) );
  
  ecstream_delete (&statement);
  
  if(mysql_errno( &(conn->handle) ))
  {
    if(mysql_errno( &(conn->handle) ))
      eclogger_log(logger, LL_ERROR, "MYSQ", mysql_error( &(conn->handle) ) );
    else
      eclogger_log(logger, LL_ERROR, "MYSQ", "unknown Mysql error" );
    
    return 0;
  }
    
  return mysql_affected_rows( &(conn->handle) );
}

/*------------------------------------------------------------------------*/

int adblmodule_dbdelete (void* ptr, AdblDelete* del, EcLogger logger)
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
  
  adbl_constructConstraint( statement, del->constraint, conn->ansi );

  eclogger_log(logger, LL_TRACE, "MYSQ", ecstream_buffer( statement ) );
    
  mysql_real_query( &(conn->handle), ecstream_buffer( statement ), ecstream_size( statement ) );
  
  if(mysql_errno( &(conn->handle) ))
  {
    if(mysql_errno( &(conn->handle) ))
      eclogger_log(logger, LL_ERROR, "MYSQ", mysql_error( &(conn->handle) ) );
    else
      eclogger_log(logger, LL_ERROR, "MYSQ", "unknown Mysql error" );
    
    return 0;
  }  
  return TRUE;
}

/*------------------------------------------------------------------------*/

void adblmodule_dbbegin( void* ptr, EcLogger logger )
{
  struct AdblMysqlConnection* conn = ptr;
  
  mysql_query(&(conn->handle), "START TRANSACTION");
}

/*------------------------------------------------------------------------*/

void adblmodule_dbcommit( void* ptr, EcLogger logger )
{
  struct AdblMysqlConnection* conn = ptr;
  
  mysql_query(&(conn->handle), "COMMIT");
}

/*------------------------------------------------------------------------*/

void adblmodule_dbrollback( void* ptr, EcLogger logger )
{
  struct AdblMysqlConnection* conn = ptr;
  
  mysql_query(&(conn->handle), "ROLLBACK");
}

/*------------------------------------------------------------------------*/

int adblmodule_dbcursor_next( void* ptr )
{
  struct AdblMysqlCursor* cursor = ptr;
  
  ecmutex_lock(cursor->mutex);
  
  cursor->row = mysql_fetch_row( cursor->res );
  cursor->pos = 0;

  ecmutex_unlock(cursor->mutex);
  
  return cursor->row != 0;
}

/*------------------------------------------------------------------------*/

const char* adblmodule_dbcursor_data (void* ptr, uint_t column)
{
  struct AdblMysqlCursor* cursor = ptr;
  if( cursor->row )
  {
    return cursor->row[column];
  }
  return "";
}

/*------------------------------------------------------------------------*/

const char* adblmodule_dbcursor_nextdata (void* ptr)
{
  struct AdblMysqlCursor* cursor = ptr;
  if( cursor->row )
  {
    const char* res = cursor->row[ cursor->pos ];
    cursor->pos++;
    return res;
  }
  return "";
}

/*------------------------------------------------------------------------*/

void adblmodule_dbcursor_release (void* ptr)
{
  struct AdblMysqlCursor* cursor = ptr;

  mysql_free_result( cursor->res );
  
  free( cursor );
}

/*------------------------------------------------------------------------*/

void* adblmodule_dbsequence_get (void* ptr, const char* table, EcLogger logger)
{
  struct AdblMysqlConnection* conn = ptr;
  
  /* try to get all columns of this table */
  MYSQL_RES* res = mysql_list_fields(&(conn->handle), table, 0);
  
  MYSQL_FIELD* field;
  
  char* column = 0;
  
  if(res == 0)
  {
    if(mysql_errno( &(conn->handle) ))
    {
      eclogger_log(logger, LL_ERROR, "MYSQ", mysql_error( &(conn->handle) ) );
    }
    else
    {
      eclogger_log(logger, LL_ERROR, "MYSQ", "unknown Mysql error" );
    }

    return 0;
  }
  /* iterate through all columns */
  for( field = mysql_fetch_field(res); field; field = mysql_fetch_field(res))
  {
    unsigned int flags = field->flags;
    
    if( (flags & PRI_KEY_FLAG) && (flags & AUTO_INCREMENT_FLAG) )
    {
      if( column )
      {
        eclogger_logformat(logger, LL_ERROR, "MYSQ", "Only one auto_increment primary key is allowed for table '%s'", table );
        /* clean up */
        mysql_free_result(res);
        return 0;
      }
      /* primary key and auto_increment */
      column = ecstr_copy (field->org_name);
    }
  }
  /* clean up */
  mysql_free_result(res);
  
  if( !column )
  {
    eclogger_logformat(logger, LL_ERROR, "MYSQ", "Please add primary key with auto_increment flag for table '%s'", table );

    return 0;
  }
  
  AdblMyslSequence* sequence = ENTC_NEW(AdblMyslSequence);

  sequence->table = ecstr_copy (table);
  sequence->conn = conn;
  sequence->column = column;
  
  return sequence;
}

/*------------------------------------------------------------------------*/

void adblmodule_dbsequence_release (void* ptr, EcLogger logger)
{
  /* casts */
  AdblMyslSequence* self = (AdblMyslSequence*)ptr;

  ecstr_delete (&(self->table));
  ecstr_delete (&(self->column));
  
  ENTC_DEL(&self, AdblMyslSequence);
}

/*------------------------------------------------------------------------*/

uint_t adblmodule_dbsequence_next (void* ptr, EcLogger logger)
{
  /* casts */
  AdblMyslSequence* sequence = (AdblMyslSequence*)ptr;
  struct AdblMysqlConnection* conn = sequence->conn;
  /* variables */
  my_ulonglong unique_id = 0;
  /* apply consitency check */
  if( !sequence ) return 0;
  /* add a fake row to the table to increase the autoincrement */
  {
    EcStream statement = ecstream_new();
    /* first disable foreign key checks */
    mysql_real_query( &(conn->handle), "SET FOREIGN_KEY_CHECKS = 0", 26 );
    
    if(mysql_errno( &(conn->handle) ))
    {
      if(mysql_errno( &(conn->handle) ))
        eclogger_log(logger, LL_ERROR, "MYSQ", mysql_error( &(conn->handle) ) );
      else
        eclogger_log(logger, LL_ERROR, "MYSQ", "unknown Mysql error" );
      
      return 0;
    }
    
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
        eclogger_log(logger, LL_ERROR, "MYSQ", mysql_error( &(conn->handle) ) );
      else
        eclogger_log(logger, LL_ERROR, "MYSQ", "unknown Mysql error" );      
    }    
  }
  /* retrieve the last inserted id */
  {
    unique_id = mysql_insert_id(&(conn->handle));
    
    if(mysql_errno( &(conn->handle) ))
    {
      if(mysql_errno( &(conn->handle) ))
        eclogger_log(logger, LL_ERROR, "MYSQ", mysql_error( &(conn->handle) ) );
      else
        eclogger_log(logger, LL_ERROR, "MYSQ", "unknown Mysql error" );
      
      return 0;
    }
    
    eclogger_logformat(logger, LL_TRACE, "MYSQ", "got new sequence number '%llu'", unique_id );
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
        eclogger_log(logger, LL_ERROR, "MYSQ", mysql_error( &(conn->handle) ) );
      else
        eclogger_log(logger, LL_ERROR, "MYSQ", "unknown Mysql error" );
      
      return 0;
    }
  }
  
  /* finally enable foreign key checks */
  mysql_real_query( &(conn->handle), "SET FOREIGN_KEY_CHECKS = 1", 26 );
  
  if(mysql_errno( &(conn->handle) ))
  {
    if(mysql_errno( &(conn->handle) ))
      eclogger_log(logger, LL_ERROR, "MYSQ", mysql_error( &(conn->handle) ) );
    else
      eclogger_log(logger, LL_ERROR, "MYSQ", "unknown Mysql error" );
    
    return 0;
  }
  
  
  return unique_id;
}

/*------------------------------------------------------------------------*/
