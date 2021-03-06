#include "adbl_sqlite3.h"

#include <system/ecfile.h>
#include <system/ecmutex.h>
#include <types/ecstream.h>
#include <types/eclist.h>
#include <types/ecintmap.h>
#include <utils/ecmessages.h>

#include "sqlite3.h"

//================================================================================================

#define MODULE "SQLT"

static const AdblModuleInfo ModuleInfo = { 10000, MODULE, "SQLite3" };

#include "adbl_module.inc"
#include "adbl_table.h"

//================================================================================================

struct AdblSqlite3Connection
{
    
  char* schema;
  
  sqlite3* handle;
  
  EcMutex mutex;
  
};

struct AdblSqlite3Cursor
{
  
  sqlite3_stmt* stmt;
  // bool
  int row;
  
  uint_t pos;
  
};

struct AdblSqliteSequence
{
    
  uint_t value;
  
};


//------------------------------------------------------------------------

void* adblmodule_dbconnect (AdblConnectionProperties* cp)
{
  struct AdblSqlite3Connection* conn = ENTC_NEW (struct AdblSqlite3Connection);
  
  conn->schema = cp->schema;
  conn->handle = 0;
  conn->mutex = ecmutex_new();

  if( cp->file )
  {
    EcString lrealpath = ecfs_getRealPath( cp->file );
    if( lrealpath )
    {
      int res = sqlite3_open(cp->file, &(conn->handle) );
      if( res == SQLITE_OK )
      {
        eclogger_fmt (LL_DEBUG, MODULE, "connect", "successful connected to Sqlite3 database '%s'", lrealpath);
      }
      else
      {
        eclogger_fmt (LL_ERROR, MODULE, "connect", "can't connect error[%i]", res);
      }
    }
    else
    {
      eclogger_fmt (LL_ERROR, MODULE, "connect", "can't resolve path '%s'", cp->file);
    }
    
    ecstr_delete( &lrealpath );
  }
  else
  {
    eclogger_msg (LL_ERROR, MODULE, "connect", "filename was not set");
  }
  
  return conn;
}

//------------------------------------------------------------------------

void adblmodule_dbdisconnect (void* ptr)
{
  struct AdblSqlite3Connection* conn = ptr;
  
  if( conn->handle )
  {
    sqlite3_close(conn->handle);
  }
  
  ecmutex_delete(&(conn->mutex));
  
  ENTC_DEL(&conn, struct AdblSqlite3Connection);
}

//------------------------------------------------------------------------

int adbl_preparexec1 (sqlite3* db, const char* statement)
{
  /* variable declaration */
  int res;
  char* errmsg;
  
  eclogger_msg (LL_TRACE, MODULE, "sql", statement);

  res = sqlite3_exec(db, statement, 0, 0, &errmsg );

  if( res != SQLITE_OK )
  {
    /* print out the sqlite3 error message */
    eclogger_fmt (LL_ERROR, MODULE, "prepare", "execute the statement: %s", errmsg);

    sqlite3_free(errmsg);    
    
    return FALSE;
  }  
  return TRUE;
}

//------------------------------------------------------------------------

int adbl_preparexec2 (sqlite3* db, EcStream statement)
{
  return adbl_preparexec1 (db, ecstream_buffer (statement));
}

/*------------------------------------------------------------------------*/

void adbl_constructListWithTable_Column (EcStream statement, AdblQueryColumn* qc, const char* table, EcIntMap orders)
{
  if( qc->table && qc->ref && qc->value )
  {
    /* add subquery */
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
  else
  {
    ecstream_append( statement, table );
    ecstream_append( statement, "." );
    ecstream_append( statement, qc->column );    
  }
  
  if( qc->orderno != 0 )
  {
    uint_t abs_orderno = abs(qc->orderno);
    
    EcBuffer buffer = ecbuf_create (10);
    ecbuf_format (buffer, 10, "C%u", abs_orderno );      
    
    ecstream_append( statement, " AS ");
    ecstream_append( statement, ecbuf_const_str (buffer) );
        
    if( qc->orderno > 0 )
    {
      ecintmap_append(orders, abs_orderno, ecstr_cat2(ecbuf_const_str (buffer), " ASC"));
    }
    else
    {
      ecintmap_append(orders, abs_orderno, ecstr_cat2(ecbuf_const_str (buffer), " DESC"));
    }
    ecbuf_destroy (&buffer);
  }
}

/*------------------------------------------------------------------------*/

void adbl_constructListWithTable( EcStream statement, EcList columns, const EcString table, EcIntMap orders )
{
  EcListNode node = eclist_first(columns);
  
  if( node != eclist_end(columns) )
  /* more than one entry */
  {
    /* first column */
    adbl_constructListWithTable_Column( statement, eclist_data(node), table, orders );
    /* next columns */
    for(node = eclist_next(node); node != eclist_end(columns); node = eclist_next(node) )
    {
      ecstream_append( statement, ", " );
      
      adbl_constructListWithTable_Column( statement, eclist_data(node), table, orders );
    }
  }
  else
  {
    ecstream_append( statement, "*" );
  }
}

/*------------------------------------------------------------------------*/

void adbl_constructContraintNode( EcStream statement, AdblConstraint* constraint );

void adbl_constructConstraintElement( EcStream statement, AdblConstraintElement* element )
{
  if( element->type == QUOMADBL_CONSTRAINT_EQUAL )
  {
    AdblConstraint* subelement = element->constraint;  
    
    if (isAssigned (subelement))
    {
      ecstream_append( statement, "(" );
      adbl_constructContraintNode (statement, subelement);
      ecstream_append( statement, ")" );
    }
    else
    {
      EcString val = ecudc_getString (element->data);
      
      ecstream_append( statement, ecudc_name(element->data) );
      ecstream_append( statement, " = \'" );
      ecstream_append( statement, val );
      ecstream_append( statement, "\'" );
      
      ecstr_delete(&val);
    }    
  }
}

//------------------------------------------------------------------------

void adbl_constructContraintNode( EcStream statement, AdblConstraint* constraint )
{
  switch (constraint->type)
  {
    case QUOMADBL_CONSTRAINT_AND:
    {
      EcListNode node = eclist_first(constraint->list);
      
      if( node != eclist_end(constraint->list) )
      {
        adbl_constructConstraintElement( statement, eclist_data(node) );
        
        node = eclist_next(node);
        
        for(; node != eclist_end(constraint->list); node = eclist_next(node) )
        {
          ecstream_append( statement, " AND " );          
          adbl_constructConstraintElement( statement, eclist_data(node) );
        }
      }      
    }
    break;
    case QUOMADBL_CONSTRAINT_IN:
    {
      EcListNode node = eclist_first (constraint->list);      
      if( node != eclist_end(constraint->list) )
      {
        AdblConstraintElement* element = eclist_data (node);
        
        // TODO
        if( element->type == QUOMADBL_CONSTRAINT_EQUAL )
        {
          {
            EcString val = ecudc_getString (element->data);
            
            ecstream_append( statement, ecudc_name(element->data) );
            ecstream_append( statement, " IN (\'" );
            ecstream_append( statement, val );
            ecstream_append( statement, "\'" );
            
            ecstr_delete(&val);
          }
            
          node = eclist_next(node);
          
          for(; node != eclist_end(constraint->list); node = eclist_next(node) )
          {
            EcString val = ecudc_getString (element->data);

            element = eclist_data (node);
            ecstream_append( statement, ",\'");
            ecstream_append( statement, val );
            ecstream_append( statement, "\'" );
            
            ecstr_delete(&val);            
          }

          ecstream_append( statement, ")" );
        }
      }            
    }
    break;
  }
}

//------------------------------------------------------------------------

void adbl_constructConstraint( EcStream statement, AdblConstraint* constraint )
{
  EcListNode node = eclist_first(constraint->list);
  
  if( node != eclist_end(constraint->list) )
  {
    ecstream_append( statement, " WHERE " );
    
    adbl_constructContraintNode( statement, constraint );
  }  
}

//------------------------------------------------------------------------

void* adblmodule_dbquery (void* ptr, AdblQuery* query)
{
  struct AdblSqlite3Connection* conn = ptr;
  /* variables */
  EcIntMap orders;
  EcStream statement;
  const char* p;
  sqlite3_stmt* stmt = 0;
  int res;

  /* check if the handle is ok */
  if( !conn->handle )
  {
    eclogger_msg (LL_ERROR, MODULE, "query", "not connected to database");    
    return 0;
  }
  
  orders = ecintmap_create (EC_ALLOC);
  
  ecmutex_lock(conn->mutex);
  
  /* create the stream */
  statement = ecstream_new();
  /* construct the stream */
  ecstream_append( statement, "SELECT " );

  adbl_constructListWithTable( statement, query->columns, query->table, orders );

  ecstream_append( statement, " FROM " );
  ecstream_append( statement, query->table );
  
  if(query->constraint)
  {
    adbl_constructConstraint( statement, query->constraint );
  }

  /* apply the order */
  ecintmap_orderAll(orders);
  
  if( ecintmap_first(orders) != ecintmap_end(orders) )
  {
    EcIntMapNode orders_node = ecintmap_first(orders);
    
    EcString alias = ecintmap_data(orders_node);
    
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
  
  ecintmap_destroy (EC_ALLOC, &orders);
  
  if(query->limit > 0)
  {
    ecstream_append( statement, " LIMIT " );
    ecstream_appendu( statement, query->limit );

    if(query->offset > 0)
    {
      ecstream_append( statement, " OFFSET " );
      ecstream_appendu( statement, query->offset );
    }
  }
  
  eclogger_msg (LL_TRACE, MODULE, "query", ecstream_buffer (statement));    
  
  res = sqlite3_prepare_v2( conn->handle,
                            ecstream_buffer( statement ),
                            ecstream_size( statement ),
                            &stmt,
                            &p );
                   
  
  ecstream_delete(&statement);
  
  if( res == SQLITE_OK )
  {
    struct AdblSqlite3Cursor* cursor = ENTC_NEW(struct AdblSqlite3Cursor);
    
    cursor->stmt = stmt;
    cursor->row = FALSE;
    cursor->pos = 0;
    
    ecmutex_unlock(conn->mutex);
    
    return cursor;
  }
  else if( res == SQLITE_ERROR )
  {
    eclogger_msg (LL_ERROR, MODULE, "query", sqlite3_errmsg(conn->handle));    
  }
	else
	{
    eclogger_fmt (LL_ERROR, MODULE, "query", "error in preparing the statement code [%i]", res);    
	}
  
  ecmutex_unlock(conn->mutex);
  return 0;
}

//------------------------------------------------------------------------------------------------------

int adblmodule_dbprocedure (void* ptr, AdblProcedure* proc)
{
  // todo: SQLITE doesn't support stored procedures
  // -> solution, just have extra implementation for stored procedures, reading external scripts etc

  return FALSE;
}

//------------------------------------------------------------------------

uint_t adblmodule_dbtable_size (void* ptr, const char* table)
{
  struct AdblSqlite3Connection* conn = ptr;
  /* variables */
  uint_t ret = 0;
  EcStream statement;
  sqlite3_stmt* stmt = 0;
  const char* p;
  int res;
  /* check if the handle is ok */
  if( !conn->handle )
  {
    eclogger_msg (LL_WARN, "SQLT", "size", "Not connected to database" );      
    
    return 0;
  }
  /* create the stream */
  statement = ecstream_new();
  /* construct the stream */
  ecstream_append( statement, "SELECT COUNT(*) FROM " );  
  ecstream_append( statement, table );

  eclogger_msg (LL_ERROR, "SQLT", "size", ecstream_buffer (statement));
  
  res = sqlite3_prepare_v2( conn->handle,
                            ecstream_buffer( statement ),
                            ecstream_size( statement ),
                            &stmt,
                            &p );
  
  ecstream_delete(&statement);
  
  if( res != SQLITE_OK )
  {
    return 0;  
  }
  
  ecmutex_lock(conn->mutex);

  res = sqlite3_step(stmt);

  ecmutex_unlock(conn->mutex);

  if( res != SQLITE_ROW )
  {
    return 0;  
  }
  
  // convert from 64bit to 32bit/64bit
  ret = (uint_t)sqlite3_column_int64(stmt, 0);
  
  sqlite3_finalize(stmt);
  
  return ret;
}

/*------------------------------------------------------------------------*/

int adbl_constructAttributesUpdate (EcStream statement, AdblAttributes* attrs)
{
  EcMapCharNode node = ecmapchar_first(attrs->columns);
  
  if( node != ecmapchar_end(attrs->columns) )
  {
    ecstream_append( statement, ecmapchar_key(node) );
    ecstream_append( statement, " = \'" );
    ecstream_append( statement, ecmapchar_data(node) );
    ecstream_append( statement, "\'" );
        
    node = ecmapchar_next(node);
    
    for(; node != ecmapchar_end(attrs->columns); node = ecmapchar_next(node) )
    {
      ecstream_append( statement, ", " );
      ecstream_append( statement, ecmapchar_key(node) );
      ecstream_append( statement, " = \'" );
      ecstream_append( statement, ecmapchar_data(node) );
      ecstream_append( statement, "\'" );
    }
    
    return TRUE;
  }
  return FALSE;
}

/*------------------------------------------------------------------------*/

void adbl_constructAttributesInsert (EcStream statement, AdblAttributes* attrs)
{
  EcMapCharNode node = ecmapchar_first(attrs->columns);
  
  if( node != ecmapchar_end(attrs->columns) )
  {
    EcStream cols = ecstream_new();
    EcStream values = ecstream_new();
    
    ecstream_append( cols, ecmapchar_key(node) );
    
    ecstream_append( values, "\"" );
    ecstream_append( values, ecmapchar_data(node) );
    ecstream_append( values, "\"" );      
    
    node = ecmapchar_next(node);
    
    for(; node != ecmapchar_end(attrs->columns); node = ecmapchar_next(node) )
    {
      ecstream_append( cols, ", " );
      ecstream_append( cols, ecmapchar_key(node) );
      
      ecstream_append( values, ", \"" );
      ecstream_append( values, ecmapchar_data(node) );
      ecstream_append( values, "\"" );      
    }
    ecstream_append( statement, " (" );
    ecstream_append( statement, ecstream_buffer( cols ) );
    ecstream_append( statement, ") VALUES (" );
    ecstream_append( statement, ecstream_buffer( values ) );
    ecstream_append( statement, ")" );
    
    ecstream_delete( &cols );
    ecstream_delete( &values );
  }
  else
  {
    ecstream_append( statement, " VALUES( NULL )" );
  }  
}

//----------------------------------------------------------------------------------

void adbl_constructAttributesInsertOrReplace (EcStream statement, AdblConstraint* constraint, AdblAttributes* attrs)
{
  EcStream cols = ecstream_new();
  EcStream values = ecstream_new();

  int cnt = 0;
  
  {
    EcListNode node;
    for (node = eclist_first (constraint->list); node != eclist_end(constraint->list); node = eclist_next(node))
    {
      AdblConstraintElement* element = eclist_data (node); 
      
      if( element->type == QUOMADBL_CONSTRAINT_EQUAL )
      {
        AdblConstraint* subelement = element->constraint;  
        if (isNotAssigned (subelement))
        {
          if (cnt > 0)
          {
            ecstream_append( cols, ", " );
            ecstream_append( values, ", " );
          }
          
          {
            EcString val = ecudc_getString (element->data);
 
            ecstream_append (cols, ecudc_name(element->data));
            ecstream_append( values, "\"" );
            ecstream_append( values, val);
            ecstream_append( values, "\"" );  

            ecstr_delete(&val);
          }
          
          cnt++;
        }
      }
    }
  }
  {
    EcMapCharNode node;
    for (node = ecmapchar_first (attrs->columns); node != ecmapchar_end(attrs->columns); node = ecmapchar_next(node))
    {
      if (cnt > 0)
      {
        ecstream_append( cols, ", " );
        ecstream_append( values, ", " );
      }

      ecstream_append( cols, ecmapchar_key(node) );
      
      ecstream_append( values, "\"" );
      ecstream_append( values, ecmapchar_data(node) );
      ecstream_append( values, "\"" );      
    }
  }
  
  ecstream_append( statement, " (" );
  ecstream_append( statement, ecstream_buffer( cols ) );
  ecstream_append( statement, ") VALUES (" );
  ecstream_append( statement, ecstream_buffer( values ) );
  ecstream_append( statement, ")" );
  
  ecstream_delete( &cols );
  ecstream_delete( &values );
}

/*------------------------------------------------------------------------*/

int adblmodule_dbupdate_insert (struct AdblSqlite3Connection* conn, AdblUpdate* update)
{
  // create the stream
  EcStream statement = ecstream_new();
  // construct the stream 
  ecstream_append( statement, "INSERT OR IGNORE INTO " );
  ecstream_append( statement, update->table );
  
  adbl_constructAttributesInsertOrReplace (statement, update->constraint, update->attrs );
  
  {
    int res = 0;
    
    if (adbl_preparexec2 (conn->handle, statement))
    {
      res = sqlite3_changes(conn->handle);
    }
    else
    {
      res = -1;
    }
    /* clean up */
    ecstream_delete( &statement );
    
    return res;
  }
  
  return 0;
}

//----------------------------------------------------------------------------------

int adblmodule_dbupdate_update (struct AdblSqlite3Connection* conn, AdblUpdate* update)
{
  EcStream statement;
  /* create the stream */
  statement = ecstream_new();
  /* construct the stream */
  ecstream_append( statement, "UPDATE " );
  ecstream_append( statement, update->table );
  ecstream_append( statement, " SET " );
  
  if (!adbl_constructAttributesUpdate(statement, update->attrs) )
  {
    ecstream_delete(&statement);
    return 0;
  }
  
  adbl_constructConstraint( statement, update->constraint );
  
  {
    int res = 0;
    
    if (adbl_preparexec2 (conn->handle, statement))
    {
      res = sqlite3_changes(conn->handle);
    }
    else
    {
      res = -1;
    }
    /* clean up */
    ecstream_delete( &statement );
    
    return res;
  }
  
  return 0;
}

//----------------------------------------------------------------------------------

int adblmodule_dbupdate (void* ptr, AdblUpdate* update, int insert)
{
  struct AdblSqlite3Connection* conn = ptr;
  /* variables */

  /* check if the handle is ok */  
  if (!conn->handle )
  {
    eclogger_msg (LL_WARN, "SQLT", "dbupdate", "Not connected to database");      
    
    return 0;
  }  
  
  if (!update->constraint)
  {
    return 0;
  }

  if (insert)
  {
    int res;

    eclogger_msg (LL_DEBUG, "SQLT", "dbupdate", "try insert or ignore");      

    res = adblmodule_dbupdate_insert (conn, update);
    if (res == 0)
    {
      return adblmodule_dbupdate_update (conn, update);
    }
    
    return res;
  }
  else
  {
    eclogger_msg (LL_DEBUG, "SQLT", "dbupdate", "try pure update");      
    
    return adblmodule_dbupdate_update (conn, update);
  }
}

//------------------------------------------------------------------------

int adblmodule_dbinsert (void* ptr, AdblInsert* insert)
{
  struct AdblSqlite3Connection* conn = ptr;
  /* variables */
  EcStream statement;

  /* check if the handle is ok */  
  if( !conn->handle )
  {
    eclogger_msg (LL_WARN, "SQLT", "dbinsert", "Not connected to database" );      
    
    return FALSE;
  }  
  /* create the stream */
  statement = ecstream_new();
  /* construct the stream */
  ecstream_append( statement, "INSERT INTO " );
  ecstream_append( statement, insert->table );
  
  adbl_constructAttributesInsert(statement, insert->attrs );
    
  {
    int res = 0;
    
    if (adbl_preparexec2 (conn->handle, statement))
    {
      res = sqlite3_changes(conn->handle);
    }
    else
    {
      res = -1;
    }
    /* clean up */
    ecstream_delete( &statement );
    
    return res;
  }
  
  return 0;
}

//------------------------------------------------------------------------

int adblmodule_dbdelete (void* ptr, AdblDelete* del)
{
  struct AdblSqlite3Connection* conn = ptr;
  /* variables */
  EcStream statement;

  /* check if the handle is ok */  
  if( !conn->handle )
  {
    eclogger_msg (LL_WARN, "SQLT", "dbdelete", "Not connected to database" );      
    
    return 0;
  }  
  
  if(!del->constraint && !del->force_all )
  {
    return 0;
  }
  
  /* create the stream */
  statement = ecstream_new();
  /* construct the stream */  
  ecstream_append( statement, "DELETE FROM " );
  ecstream_append( statement, del->table );
  
  if( del->constraint )
  {
    adbl_constructConstraint( statement, del->constraint );    
  }
  
  
  {
    int res = 0;
    
    if (adbl_preparexec2 (conn->handle, statement))
    {
      res = sqlite3_changes(conn->handle);
    }
    else
    {
      res = -1;
    }
    /* clean up */
    ecstream_delete( &statement );
    
    return res;
  }
  
  return 0;
}

//------------------------------------------------------------------------

void adblmodule_dbbegin (void* ptr)
{  
  struct AdblSqlite3Connection* conn = ptr;
  /* check if the handle is ok */  
  if( !conn->handle )
  {
    eclogger_msg (LL_WARN, "SQLT", "dbbegin", "Not connected to database" );      
    
    return;
  }
  
  adbl_preparexec1 (conn->handle, "BEGIN");
}

//------------------------------------------------------------------------

void adblmodule_dbcommit (void* ptr)
{
  struct AdblSqlite3Connection* conn = ptr;
  /* check if the handle is ok */  
  if( !conn->handle )
  {
    eclogger_msg (LL_WARN, "SQLT", "dbcommit", "Not connected to database" );      
    
    return;
  }  
  
  adbl_preparexec1 (conn->handle, "COMMIT");
}

//------------------------------------------------------------------------

void adblmodule_dbrollback ( void* ptr)
{
  struct AdblSqlite3Connection* conn = ptr;
  /* check if the handle is ok */  
  if( !conn->handle )
  {
    eclogger_msg (LL_WARN, "SQLT", "dbrollback", "Not connected to database" );      
    
    return;
  }
  
  adbl_preparexec1 (conn->handle, "ROLLBACK");
}

/*------------------------------------------------------------------------*/

int adblmodule_dbcursor_next (void* ptr)
{
  struct AdblSqlite3Cursor* cursor = ptr;
  /* reset the position */
  cursor->pos = 0;
  /* iterate */
  cursor->row = (sqlite3_step(cursor->stmt) == SQLITE_ROW);
  
  return cursor->row;
}

/*------------------------------------------------------------------------*/

const char* adblmodule_dbcursor_data (void* ptr, uint_t column)
{
  struct AdblSqlite3Cursor* cursor = ptr;

  if( cursor->row )
    return (const char*)sqlite3_column_text(cursor->stmt, column);
  
  return 0;
}

/*------------------------------------------------------------------------*/

const char* adblmodule_dbcursor_nextdata (void* ptr)
{
  struct AdblSqlite3Cursor* cursor = ptr;
  
  if( cursor->row )
  {
    const char* data = (const char*)sqlite3_column_text(cursor->stmt, cursor->pos );
    cursor->pos++;

    return data;
  }
  return 0;
}

/*------------------------------------------------------------------------*/

void adblmodule_dbcursor_release (void* ptr)
{
  struct AdblSqlite3Cursor* cursor = ptr;
  
  sqlite3_finalize(cursor->stmt);
  
  ENTC_DEL (&cursor, struct AdblSqlite3Cursor);
}

/*------------------------------------------------------------------------*/

void* adblmodule_dbsequence_get (void* ptr, const char* table)
{
  /* cast */
  struct AdblSqlite3Connection* conn = ptr;
  /* variables */
  struct AdblSqliteSequence* self = 0;
  int res;
  const char* p;
  sqlite3_stmt* stmt;
  EcString column = 0;
  /* create a new query to get table informations */
  EcStream statement; 
  
  if( !conn->handle )
  {
    eclogger_msg (LL_WARN, "SQLT", "dbsequence", "Not connected to database" );      
    
    return 0;
  }
  
  statement = ecstream_new();
  
  /* construct the stream */
  ecstream_append( statement, "PRAGMA TABLE_INFO(" );  
  ecstream_append( statement, table );
  ecstream_append( statement, ")" );

  eclogger_msg (LL_TRACE, "SQLT", "dbsequence", ecstream_buffer (statement));
  
  
  res = sqlite3_prepare_v2( conn->handle,
                           ecstream_buffer( statement ),
                           ecstream_size( statement ),
                           &stmt,
                           &p );
  
  /* clean */
  ecstream_delete( &statement );
  
  if (res != SQLITE_OK)
  {
    eclogger_msg (LL_ERROR, "SQLT", "dbsequence", "Error in last statement");
    return 0;  
  }
  
  ecmutex_lock(conn->mutex);
  res = sqlite3_step(stmt);
  ecmutex_unlock(conn->mutex);
  
  while( res == SQLITE_ROW )
  {
    int pk = sqlite3_column_int(stmt, 5);
    
    if(pk == 1)
    {
      column = ecstr_copy((const char*)sqlite3_column_text(stmt, 1));
      
      break;
    }
    /* get next row */
    ecmutex_lock(conn->mutex);
    res = sqlite3_step(stmt);
    ecmutex_unlock(conn->mutex);
  }
  
  res = sqlite3_finalize(stmt);

  if (res != SQLITE_OK)
  {
    eclogger_msg (LL_ERROR, "SQLT", "dbsequence", "Error in finalize");

    ecstr_delete(&column);
    
    return 0;  
  }
  
  if (!ecstr_valid(column))
  {
    /* create a new instance */
    self = ENTC_NEW(struct AdblSqliteSequence);
    /* init value */
    self->value = 0;
    
    return self;
  }
  
  /* create new statement to get the last value */
  statement = ecstream_new();
  /* construct the stream */
  ecstream_append( statement, "SELECT MAX(" );  
  ecstream_append( statement, column );
  ecstream_append( statement, ") FROM " );
  ecstream_append( statement, table );
  
  eclogger_msg (LL_TRACE, "SQLT", "dbsequence", ecstream_buffer (statement));
  
  res = sqlite3_prepare_v2( conn->handle,
                           ecstream_buffer( statement ),
                           ecstream_size( statement ),
                           &stmt,
                           &p );
  
  /* clean */
  ecstream_delete( &statement );
  ecstr_delete( &column );
  
  if( res != SQLITE_OK )
  {
    eclogger_msg (LL_TRACE, "SQLT", "dbsequence", "fetch failed");
    return 0;
  }

  /* create a new instance */
  self = ENTC_NEW(struct AdblSqliteSequence);
  /* init */
  self->value = 0;
  
  ecmutex_lock(conn->mutex);
  res = sqlite3_step(stmt);
  ecmutex_unlock(conn->mutex);

  if( res == SQLITE_ROW )
  {
    self->value = (uint_t)sqlite3_column_int64(stmt, 0);
  }
  
  sqlite3_finalize(stmt);

  eclogger_msg (LL_TRACE, "SQLT", "dbsequence", "fetch done #3");

  return self;
}

/*------------------------------------------------------------------------*/

void adblmodule_dbsequence_release (void* ptr)
{
  /* cast */
  struct AdblSqliteSequence* self = ptr;

  ENTC_DEL (&self, struct AdblSqliteSequence);
}

/*------------------------------------------------------------------------*/

uint_t adblmodule_dbsequence_next (void* ptr)
{
  /* cast */
  struct AdblSqliteSequence* self = ptr;
  
  if( self )
  {
    self->value++;

    return self->value;
  }
  
  return 0;
}

//----------------------------------------------------------------------------------------

EcList adblmodule_dbschema (void* ptr)
{
  // variables
  EcStream statement; 
  int res;
  const char* p;
  sqlite3_stmt* stmt;
  EcList ret;
  // cast
  struct AdblSqlite3Connection* conn = ptr;
  
  if( !conn->handle )
  {
    eclogger_msg (LL_WARN, "SQLT", "dbschema", "Not connected to database");      
    
    return 0;
  }
  
  statement = ecstream_new ();
  
  ecstream_append (statement, "SELECT name FROM sqlite_master WHERE type='table'");  

  eclogger_msg (LL_TRACE, "SQLT", "dbschema", ecstream_buffer (statement));
  
  res = sqlite3_prepare_v2 (conn->handle,
                            ecstream_buffer( statement ),
                            ecstream_size( statement ),
                            &stmt,
                            &p);
  
  // clean up
  ecstream_delete (&statement);
  
  if( res != SQLITE_OK )
  {
    eclogger_msg (LL_ERROR, "SQLT", "dbschema", "Error in last statement");
    
    return 0;  
  } 
  
  ecmutex_lock(conn->mutex);
  res = sqlite3_step(stmt);
  ecmutex_unlock(conn->mutex);
  
  // so far so good
  ret = eclist_create_ex (EC_ALLOC);
  
  while( res == SQLITE_ROW )
  {
    eclist_append (ret, ecstr_copy((const char*)sqlite3_column_text(stmt, 0)));
    // get next row
    ecmutex_lock(conn->mutex);
    res = sqlite3_step(stmt);
    ecmutex_unlock(conn->mutex);
  }
  
  res = sqlite3_finalize(stmt);
  
  if( res != SQLITE_OK )
  {
    eclogger_msg (LL_ERROR, "SQLT", "dbschema", "Error in finalize" );
    
    adbl_schema_del (&ret);
    
    return 0;  
  }  
  
  return ret;
}

//----------------------------------------------------------------------------------------

void adblmodule_parseColumn (AdblTable* table, const EcString statement)
{
  EcListNode node;
  EcList list = eclist_create_ex (EC_ALLOC);
  
  EcString column;

  ecstr_tokenizer(list, statement, ' ');
  
  node = eclist_first(list);
  
  if (node == eclist_end(list))
  {
    ecstr_tokenizer_clear (list);
    eclist_free_ex (EC_ALLOC, &list);
    
    return;
  }
  
  column = ecstr_trim (eclist_data (node));

  node = eclist_next(node);
  if (node == eclist_end(list))
  {
    ecstr_tokenizer_clear (list);
    eclist_free_ex (EC_ALLOC, &list);
    ecstr_delete (&column);
    
    return;
  }
  
  node = eclist_next(node);
  if (node == eclist_end(list))
  {
    eclogger_fmt (LL_TRACE, "SQLT", "dbtable", "added column: %s", column);          

    eclist_append (table->columns, column);
  }
  else
  {
    if (ecstr_equal(eclist_data (node), "PRIMARY"))
    {
      eclogger_fmt (LL_TRACE, "SQLT", "dbtable", "added primary key: %s", column);          
      
      eclist_append (table->primary_keys, column);      
    }
    else
    {
      ecstr_delete(&column);
    }
  }

  ecstr_tokenizer_clear (list);
  eclist_free_ex (EC_ALLOC, &list);
}

//----------------------------------------------------------------------------------------

void adblmodule_parseForeignKey (AdblTable* table, const EcString statement)
{
  
  EcListNode node;
  EcList list = eclist_create_ex (EC_ALLOC);
  
  EcString column;
  EcString tablename;
  EcString reference;
  
  ecstr_tokenizer(list, statement, ' ');

  node = eclist_first(list);
  
  if (node == eclist_end(list))
  {
    ecstr_tokenizer_clear (list);
    eclist_free_ex (EC_ALLOC,&list);
    
    return;
  }
  
  column = ecstr_shrink (eclist_data (node), '(', ')');

  node = eclist_next(node);
  if (node == eclist_end(list))
  {
    ecstr_tokenizer_clear (list);
    eclist_free_ex (EC_ALLOC, &list);
    ecstr_delete (&column);
    
    return;
  }
  
  node = eclist_next(node);
  if (node == eclist_end(list))
  {
    ecstr_tokenizer_clear (list);
    eclist_free_ex (EC_ALLOC, &list);
    ecstr_delete (&column);
    
    return;
  }

  tablename = ecstr_extractf (eclist_data (node), '(');
  reference = ecstr_shrink (eclist_data (node), '(', ')');
  
  eclogger_fmt (LL_TRACE, "SQLT", "dbtable", "add foreign key: %s with reference %s.%s", column, tablename, reference);          
  
  {
    AdblForeignKeyConstraint* fkconstraint = ENTC_NEW (AdblForeignKeyConstraint);
    
    fkconstraint->name = table->name;
    fkconstraint->column_name = column;
    fkconstraint->table = tablename;
    fkconstraint->reference = reference;
    
    eclist_append (table->foreign_keys, fkconstraint);
  }
  
  ecstr_tokenizer_clear (list);
  eclist_free_ex (EC_ALLOC, &list);
}

//----------------------------------------------------------------------------------------

AdblTable* adblmodule_parseCreateStatement (const EcString tablename, const EcString statement)
{
  EcListNode node;
  EcList list = eclist_create_ex (EC_ALLOC);
  
  AdblTable* table = adbl_table_new (tablename);
    
  EcString s1 = ecstr_shrink (statement, '(', ')');
  
  ecstr_tokenizer(list, s1, ',');
  
  for (node = eclist_first(list); node != eclist_end(list); node = eclist_next(node))
  {
    EcString part = eclist_data(node);
    EcString token = ecstr_trim (part);
    // check for foreign keys
    if (ecstr_leading(token, "FOREIGN KEY "))
    {
      adblmodule_parseForeignKey (table, token + 12);
    }
    else
    {
      adblmodule_parseColumn (table, token);
    }
    ecstr_delete(&token);
    ecstr_delete(&part);
  }
  
  eclist_free_ex (EC_ALLOC, &list);
  
  ecstr_delete(&s1);
  
  return table;
}

//----------------------------------------------------------------------------------------

AdblTable* adblmodule_dbtable (void* ptr, const EcString tablename)
{
  // variables
  EcStream statement; 
  int res;
  const char* p;
  sqlite3_stmt* stmt;
  AdblTable* ret = NULL;
  // cast
  struct AdblSqlite3Connection* conn = ptr;
  
  if( !conn->handle )
  {
    eclogger_msg (LL_WARN, "SQLT", "dbtable", "Not connected to database" );      
    
    return ret;
  }
  
  statement = ecstream_new ();
  
  ecstream_append (statement, "SELECT sql FROM sqlite_master WHERE type = 'table' and name='");
  ecstream_append (statement, tablename);
  ecstream_append (statement, "'");  
  
  eclogger_msg (LL_TRACE, "SQLT", "dbtable", ecstream_buffer (statement));
  
  res = sqlite3_prepare_v2 (conn->handle,
                            ecstream_buffer( statement ),
                            ecstream_size( statement ),
                            &stmt,
                            &p);
  
  // clean up
  ecstream_delete (&statement);
  
  if (res != SQLITE_OK)
  {
    eclogger_msg (LL_ERROR, "SQLT", "dbtable", "Error in last statement");    
    return 0;  
  } 
  
  ecmutex_lock(conn->mutex);
  res = sqlite3_step(stmt);
  ecmutex_unlock(conn->mutex);
  
  if (res == SQLITE_ROW)
  {
    ret = adblmodule_parseCreateStatement (tablename, (const char*)sqlite3_column_text(stmt, 0));
  }
  
  res = sqlite3_finalize(stmt);
  
  if( res != SQLITE_OK )
  {
    eclogger_msg (LL_ERROR, "SQLT", "dbtable", "Error in finalize" );
    
    adbl_table_del (&ret);
    
    return 0;  
  }  
  
  return ret;
}

//----------------------------------------------------------------------------------------
