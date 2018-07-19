#include "adbl_sqlite3.h"

#include <system/ecfile.h>
#include <system/ecmutex.h>
#include <types/ecstream.h>
#include <types/eclist.h>
#include <tools/eclog.h>
#include <tools/ectokenizer.h>

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
        eclog_fmt (LL_DEBUG, MODULE, "connect", "successful connected to Sqlite3 database '%s'", lrealpath);
      }
      else
      {
        eclog_fmt (LL_ERROR, MODULE, "connect", "can't connect error[%i]", res);
      }
    }
    else
    {
      eclog_fmt (LL_ERROR, MODULE, "connect", "can't resolve path '%s'", cp->file);
    }
    
    ecstr_delete( &lrealpath );
  }
  else
  {
    eclog_msg (LL_ERROR, MODULE, "connect", "filename was not set");
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
  
  eclog_msg (LL_TRACE, MODULE, "sql", statement);

  res = sqlite3_exec(db, statement, 0, 0, &errmsg );

  if( res != SQLITE_OK )
  {
    /* print out the sqlite3 error message */
    eclog_fmt (LL_ERROR, MODULE, "prepare", "execute the statement: %s", errmsg);

    sqlite3_free(errmsg);    
    
    return FALSE;
  }  
  return TRUE;
}

//------------------------------------------------------------------------

int adbl_preparexec2 (sqlite3* db, EcStream statement)
{
  return adbl_preparexec1 (db, ecstream_get (statement));
}

/*------------------------------------------------------------------------*/

void adbl_constructListWithTable_Column (EcStream statement, AdblQueryColumn* qc, const char* table, EcMap orders)
{
  if( qc->table && qc->ref && qc->value )
  {
    /* add subquery */
    ecstream_append_str( statement, "( SELECT " );
    ecstream_append_str( statement, qc->table );
    ecstream_append_str( statement, "." );
    ecstream_append_str( statement, qc->value );
    ecstream_append_str( statement, " FROM " );
    ecstream_append_str( statement, qc->table );
    ecstream_append_str( statement, " WHERE " );
    ecstream_append_str( statement, qc->table );
    ecstream_append_str( statement, "." );
    ecstream_append_str( statement, qc->ref );
    ecstream_append_str( statement, " = " );
    ecstream_append_str( statement, table );
    ecstream_append_str( statement, "." );
    ecstream_append_str( statement, qc->column );
    ecstream_append_str( statement, " )" );
  }
  else
  {
    ecstream_append_str( statement, table );
    ecstream_append_str( statement, "." );
    ecstream_append_str( statement, qc->column );
  }
  
  if( qc->orderno != 0 )
  {
    int64_t abs_orderno = abs(qc->orderno);
    
    EcBuffer buffer = ecbuf_create (10);
    ecbuf_format (buffer, 10, "C%u", abs_orderno );      
    
    ecstream_append_str( statement, " AS ");
    ecstream_append_str( statement, ecbuf_const_str (buffer) );
        
    if( qc->orderno > 0 )
    {
      ecmap_insert (orders, (void*)abs_orderno, ecstr_cat2(ecbuf_const_str (buffer), " ASC"));
    }
    else
    {
      ecmap_insert (orders, (void*)abs_orderno, ecstr_cat2(ecbuf_const_str (buffer), " DESC"));
    }
    
    ecbuf_destroy (&buffer);
  }
}

/*------------------------------------------------------------------------*/

void adbl_constructListWithTable( EcStream statement, EcList columns, const EcString table, EcMap orders )
{
  EcListCursor cursor;
  eclist_cursor_init (columns, &cursor, LIST_DIR_NEXT);
  
  if (eclist_cursor_next (&cursor))
  {
    /* first column */
    adbl_constructListWithTable_Column (statement, eclist_data (cursor.node), table, orders);
    /* next columns */
    
    while (eclist_cursor_next (&cursor))
    {
      ecstream_append_str( statement, ", " );
      
      adbl_constructListWithTable_Column (statement, eclist_data (cursor.node), table, orders);
    }
  }
  else
  {
    ecstream_append_str( statement, "*" );
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
      ecstream_append_str( statement, "(" );
      adbl_constructContraintNode (statement, subelement);
      ecstream_append_str( statement, ")" );
    }
    else
    {
      EcString val = ecudc_getString (element->data);
      
      ecstream_append_str( statement, ecudc_name(element->data) );
      ecstream_append_str( statement, " = \'" );
      ecstream_append_str( statement, val );
      ecstream_append_str( statement, "\'" );
      
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
      EcListCursor cursor;
      eclist_cursor_init (constraint->list, &cursor, LIST_DIR_NEXT);
      
      if (eclist_cursor_next (&cursor))
      {
        adbl_constructConstraintElement (statement, eclist_data (cursor.node));
        
        while (eclist_cursor_next (&cursor))
        {
          ecstream_append_str (statement, " AND ");
          adbl_constructConstraintElement (statement, eclist_data (cursor.node));
        }
      }
      
      break;
    }
    case QUOMADBL_CONSTRAINT_IN:
    {
      EcListCursor cursor;
      eclist_cursor_init (constraint->list, &cursor, LIST_DIR_NEXT);

      if (eclist_cursor_next (&cursor))
      {
        AdblConstraintElement* element = eclist_data (cursor.node);
        
        // TODO
        if( element->type == QUOMADBL_CONSTRAINT_EQUAL )
        {
          {
            EcString val = ecudc_getString (element->data);
            
            ecstream_append_str( statement, ecudc_name(element->data) );
            ecstream_append_str( statement, " IN (\'" );
            ecstream_append_str( statement, val );
            ecstream_append_str( statement, "\'" );
            
            ecstr_delete(&val);
          }
          
          while (eclist_cursor_next (&cursor))
          {
            EcString val = ecudc_getString (element->data);

            element = eclist_data (cursor.node);
            ecstream_append_str( statement, ",\'");
            ecstream_append_str( statement, val );
            ecstream_append_str( statement, "\'" );
            
            ecstr_delete(&val);            
          }

          ecstream_append_str( statement, ")" );
        }
      }            
    }
    break;
  }
}

//------------------------------------------------------------------------

void adbl_constructConstraint( EcStream statement, AdblConstraint* constraint )
{
  if (eclist_hasContent (constraint->list))
  {
    ecstream_append_str( statement, " WHERE " );
    
    adbl_constructContraintNode( statement, constraint );
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

//------------------------------------------------------------------------

void* adblmodule_dbquery (void* ptr, AdblQuery* query)
{
  struct AdblSqlite3Connection* conn = ptr;
  /* variables */
  EcMap orders;
  EcStream statement;
  const char* p;
  sqlite3_stmt* stmt = 0;
  int res;

  /* check if the handle is ok */
  if( !conn->handle )
  {
    eclog_msg (LL_ERROR, MODULE, "query", "not connected to database");    
    return 0;
  }
  
  orders = ecmap_create (adblmodule_dbquery_orders_onCmp, adblmodule_dbquery_orders_onDel);
  
  ecmutex_lock(conn->mutex);
  
  /* create the stream */
  statement = ecstream_create ();
  /* construct the stream */
  ecstream_append_str (statement, "SELECT ");

  adbl_constructListWithTable (statement, query->columns, query->table, orders);

  ecstream_append_str (statement, " FROM ");
  ecstream_append_str (statement, query->table);
  
  if(query->constraint)
  {
    adbl_constructConstraint (statement, query->constraint);
  }

  // apply orders
  {
    EcMapCursor cursor;
    ecmap_cursor_init (orders, &cursor, LIST_DIR_NEXT);

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
    ecstream_append_str( statement, " LIMIT " );
    ecstream_append_u( statement, query->limit );

    if(query->offset > 0)
    {
      ecstream_append_str( statement, " OFFSET " );
      ecstream_append_u( statement, query->offset );
    }
  }
  
  //eclog_msg (LL_TRACE, MODULE, "query", ecstream_get (statement));
  
  res = sqlite3_prepare_v2( conn->handle,
                            ecstream_get( statement ),
                            ecstream_size( statement ),
                            &stmt,
                            &p );
                   
  
  ecstream_destroy(&statement);
  
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
    eclog_msg (LL_ERROR, MODULE, "query", sqlite3_errmsg(conn->handle));    
  }
  else
  {
    eclog_fmt (LL_ERROR, MODULE, "query", "error in preparing the statement code [%i]", res);    
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
    eclog_msg (LL_WARN, "SQLT", "size", "Not connected to database" );    
    return 0;
  }
  /* create the stream */
  statement = ecstream_create ();
  /* construct the stream */
  ecstream_append_str (statement, "SELECT COUNT(*) FROM ");
  ecstream_append_str (statement, table);

  res = sqlite3_prepare_v2 (conn->handle, ecstream_get (statement), ecstream_size (statement), &stmt, &p);
  
  ecstream_destroy (&statement);
  
  if (res != SQLITE_OK)
  {
    // TODO: error handling
    return 0;  
  }
  
  ecmutex_lock(conn->mutex);

  res = sqlite3_step (stmt);

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

void adbl_constructAttributesUpdate (EcStream statement, AdblAttributes* attrs)
{
  EcMapCursor cursor; ecmap_cursor_init (attrs->columns, &cursor, LIST_DIR_NEXT);

  // iterate through all columns
  while (ecmap_cursor_next (&cursor))
  {
    EcMapNode node = cursor.node;
    
    if (cursor.position > 0)
    {
      ecstream_append_str (statement, ", ");
    }

    ecstream_append_str (statement, ecmap_node_key (node));
    ecstream_append_str( statement, " = \'" );
    ecstream_append_str( statement, ecmap_node_value (node));
    ecstream_append_str( statement, "\'" );
  }  
}

/*------------------------------------------------------------------------*/

void adbl_constructAttributesInsert (EcStream statement, AdblAttributes* attrs)
{
  EcMapCursor cursor;

  EcStream cols = ecstream_create();
  EcStream values = ecstream_create();

  ecmap_cursor_init (attrs->columns, &cursor, LIST_DIR_NEXT);

  // iterate through all columns
  while (ecmap_cursor_next (&cursor))
  {
    EcMapNode node = cursor.node;
    
    if (cursor.position > 0)
    {
      ecstream_append_c (cols, ',');
      ecstream_append_c (values, ',');
    }
    
    ecstream_append_str (cols, ecmap_node_key (node));
    
    ecstream_append_c (values, '"');
    ecstream_append_str (values, ecmap_node_value (node));
    ecstream_append_c (values, '"');
  }

  if (ecstream_size (cols) > 0)
  {
    ecstream_append_str (statement, " (");
    ecstream_append_stream (statement, cols);
    ecstream_append_str (statement, ") VALUES (");
    ecstream_append_stream (statement, values);
    ecstream_append_c (statement, ')');
  }
  else
  {
    ecstream_append_str (statement, " VALUES( NULL )");    
  }
    
  ecstream_destroy (&cols);
  ecstream_destroy (&values);
}

//----------------------------------------------------------------------------------

void adbl_constructAttributesInsertOrReplace (EcStream statement, AdblConstraint* constraint, AdblAttributes* attrs)
{
  EcStream cols = ecstream_create();
  EcStream values = ecstream_create();

  int cnt = 0;
  
  {
    EcListCursor cursor;
    eclist_cursor_init (constraint->list, &cursor, LIST_DIR_NEXT);
    
    while (eclist_cursor_next (&cursor))
    {
      AdblConstraintElement* element = eclist_data (cursor.node);
      
      if( element->type == QUOMADBL_CONSTRAINT_EQUAL )
      {
        AdblConstraint* subelement = element->constraint;  
        if (isNotAssigned (subelement))
        {
          if (cnt > 0)
          {
            ecstream_append_str( cols, ", " );
            ecstream_append_str( values, ", " );
          }
          
          {
            EcString val = ecudc_getString (element->data);
 
            ecstream_append_str (cols, ecudc_name(element->data));
            ecstream_append_str( values, "\"" );
            ecstream_append_str( values, val);
            ecstream_append_str( values, "\"" );

            ecstr_delete(&val);
          }
          
          cnt++;
        }
      }
    }
  }
  {
    EcMapCursor cursor; ecmap_cursor_init (attrs->columns, &cursor, LIST_DIR_NEXT);

    // iterate through all columns
    while (ecmap_cursor_next (&cursor))
    {
      EcMapNode node = cursor.node;
      
      if (cnt > 0)
      {
        ecstream_append_str (cols, ", ");
        ecstream_append_str (values, ", ");
      }

      ecstream_append_str (cols, ecmap_node_key (node));
      
      ecstream_append_str (values, "\"");
      ecstream_append_str (values, ecmap_node_value (node));
      ecstream_append_str (values, "\"");
    }
  }
  
  ecstream_append_str( statement, " (" );
  ecstream_append_stream( statement, cols);
  ecstream_append_str( statement, ") VALUES (" );
  ecstream_append_stream( statement, values);
  ecstream_append_str( statement, ")" );
  
  ecstream_destroy( &cols );
  ecstream_destroy( &values );
}

/*------------------------------------------------------------------------*/

int adblmodule_dbupdate_insert (struct AdblSqlite3Connection* conn, AdblUpdate* update)
{
  // create the stream
  EcStream statement = ecstream_create ();
  // construct the stream 
  ecstream_append_str( statement, "INSERT OR IGNORE INTO " );
  ecstream_append_str( statement, update->table );
  
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
    ecstream_destroy( &statement );
    
    return res;
  }
  
  return 0;
}

//----------------------------------------------------------------------------------

int adblmodule_dbupdate_update (struct AdblSqlite3Connection* conn, AdblUpdate* update)
{
  EcStream statement;
  /* create the stream */
  statement = ecstream_create ();
  /* construct the stream */
  ecstream_append_str( statement, "UPDATE " );
  ecstream_append_str( statement, update->table );
  ecstream_append_str( statement, " SET " );
  
  adbl_constructAttributesUpdate (statement, update->attrs);
  
  adbl_constructConstraint (statement, update->constraint);
  
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
    ecstream_destroy( &statement );
    
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
    eclog_msg (LL_WARN, "SQLT", "dbupdate", "Not connected to database");      
    
    return 0;
  }  
  
  if (!update->constraint)
  {
    return 0;
  }

  if (insert)
  {
    int res;

    eclog_msg (LL_DEBUG, "SQLT", "dbupdate", "try insert or ignore");      

    res = adblmodule_dbupdate_insert (conn, update);
    if (res == 0)
    {
      return adblmodule_dbupdate_update (conn, update);
    }
    
    return res;
  }
  else
  {
    eclog_msg (LL_DEBUG, "SQLT", "dbupdate", "try pure update");      
    
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
    eclog_msg (LL_WARN, "SQLT", "dbinsert", "Not connected to database" );      
    
    return FALSE;
  }  
  /* create the stream */
  statement = ecstream_create();
  /* construct the stream */
  ecstream_append_str( statement, "INSERT INTO " );
  ecstream_append_str( statement, insert->table );
  
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
    ecstream_destroy( &statement );
    
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
    eclog_msg (LL_WARN, "SQLT", "dbdelete", "Not connected to database" );      
    
    return 0;
  }  
  
  if(!del->constraint && !del->force_all )
  {
    return 0;
  }
  
  /* create the stream */
  statement = ecstream_create();
  /* construct the stream */  
  ecstream_append_str( statement, "DELETE FROM " );
  ecstream_append_str( statement, del->table );
  
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
    ecstream_destroy( &statement );
    
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
    eclog_msg (LL_WARN, "SQLT", "dbbegin", "Not connected to database" );      
    
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
    eclog_msg (LL_WARN, "SQLT", "dbcommit", "Not connected to database" );      
    
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
    eclog_msg (LL_WARN, "SQLT", "dbrollback", "Not connected to database" );      
    
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
    eclog_msg (LL_WARN, "SQLT", "dbsequence", "Not connected to database" );      
    
    return 0;
  }
  
  statement = ecstream_create();
  
  /* construct the stream */
  ecstream_append_str( statement, "PRAGMA TABLE_INFO(" );
  ecstream_append_str( statement, table );
  ecstream_append_str( statement, ")" );

  eclog_msg (LL_TRACE, "SQLT", "dbsequence", ecstream_get (statement));
  
  
  res = sqlite3_prepare_v2( conn->handle,
                           ecstream_get( statement ),
                           ecstream_size( statement ),
                           &stmt,
                           &p );
  
  /* clean */
  ecstream_destroy( &statement );
  
  if (res != SQLITE_OK)
  {
    eclog_msg (LL_ERROR, "SQLT", "dbsequence", "Error in last statement");
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
    eclog_msg (LL_ERROR, "SQLT", "dbsequence", "Error in finalize");

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
  statement = ecstream_create ();
  /* construct the stream */
  ecstream_append_str( statement, "SELECT MAX(" );
  ecstream_append_str( statement, column );
  ecstream_append_str( statement, ") FROM " );
  ecstream_append_str( statement, table );
  
  eclog_msg (LL_TRACE, "SQLT", "dbsequence", ecstream_get (statement));
  
  res = sqlite3_prepare_v2( conn->handle,
                           ecstream_get( statement ),
                           ecstream_size( statement ),
                           &stmt,
                           &p );
  
  /* clean */
  ecstream_destroy( &statement );
  ecstr_delete( &column );
  
  if( res != SQLITE_OK )
  {
    eclog_msg (LL_TRACE, "SQLT", "dbsequence", "fetch failed");
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

  eclog_msg (LL_TRACE, "SQLT", "dbsequence", "fetch done #3");

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

//-----------------------------------------------------------------------------

static int __STDCALL adblmodule_dbschema_onDestroy (void* ptr)
{
  EcString h = ptr;
  ecstr_delete (&h);
  
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
    eclog_msg (LL_WARN, "SQLT", "dbschema", "Not connected to database");      
    
    return 0;
  }
  
  statement = ecstream_create ();
  
  ecstream_append_str (statement, "SELECT name FROM sqlite_master WHERE type='table'");

  eclog_msg (LL_TRACE, "SQLT", "dbschema", ecstream_get (statement));
  
  res = sqlite3_prepare_v2 (conn->handle,
                            ecstream_get( statement ),
                            ecstream_size( statement ),
                            &stmt,
                            &p);
  
  // clean up
  ecstream_destroy (&statement);
  
  if( res != SQLITE_OK )
  {
    eclog_msg (LL_ERROR, "SQLT", "dbschema", "Error in last statement");
    
    return 0;  
  } 
  
  ecmutex_lock(conn->mutex);
  res = sqlite3_step(stmt);
  ecmutex_unlock(conn->mutex);
  
  // so far so good
  ret = eclist_create (adblmodule_dbschema_onDestroy);
  
  while( res == SQLITE_ROW )
  {
    eclist_push_back (ret, ecstr_copy((const char*)sqlite3_column_text(stmt, 0)));
    // get next row
    ecmutex_lock(conn->mutex);
    res = sqlite3_step(stmt);
    ecmutex_unlock(conn->mutex);
  }
  
  res = sqlite3_finalize(stmt);
  
  if( res != SQLITE_OK )
  {
    eclog_msg (LL_ERROR, "SQLT", "dbschema", "Error in finalize" );
    
    eclist_destroy (&ret);
    return 0;
  }  
  
  return ret;
}

//----------------------------------------------------------------------------------------

void adblmodule_parseColumn (AdblTable* table, const EcString statement)
{
  EcString column;
  EcList list = ectokenizer_parse (statement, ' ');
  
  EcListCursor cursor;
  eclist_cursor_init (list, &cursor, LIST_DIR_NEXT);
  
  if (!eclist_cursor_next (&cursor))
  {
    eclist_destroy (&list);
    return;
  }
  
  column = ecstr_trim (eclist_data (cursor.node));

  if (!eclist_cursor_next (&cursor))
  {
    eclist_destroy (&list);
    ecstr_delete (&column);
    return;
  }
  
  if (eclist_cursor_next (&cursor))
  {
    if (ecstr_equal(eclist_data (cursor.node), "PRIMARY"))
    {
      eclog_fmt (LL_TRACE, "SQLT", "dbtable", "added primary key: %s", column);
      
      eclist_push_back (table->primary_keys, column);
    }
    else
    {
      ecstr_delete(&column);
    }
  }
  else
  {
    eclog_fmt (LL_TRACE, "SQLT", "dbtable", "added column: %s", column);
    
    eclist_push_back (table->columns, column);
  }
  
  eclist_destroy (&list);
}

//----------------------------------------------------------------------------------------

void adblmodule_parseForeignKey (AdblTable* table, const EcString statement)
{
  EcString column;
  EcString tablename;
  EcString reference;
  
  EcList list = ectokenizer_parse (statement, ' ');

  EcListCursor cursor;
  eclist_cursor_init (list, &cursor, LIST_DIR_NEXT);
  
  if (!eclist_cursor_next (&cursor))
  {
    eclist_destroy (&list);
    return;
  }

  column = ecstr_shrink (eclist_data (cursor.node), '(', ')');

  if (!eclist_cursor_next (&cursor))
  {
    eclist_destroy (&list);
    ecstr_delete (&column);
    return;
  }

  if (!eclist_cursor_next (&cursor))
  {
    eclist_destroy (&list);
    ecstr_delete (&column);
    return;
  }

  tablename = ecstr_extractf (eclist_data (cursor.node), '(');
  reference = ecstr_shrink (eclist_data (cursor.node), '(', ')');
  
  eclog_fmt (LL_TRACE, "SQLT", "dbtable", "add foreign key: %s with reference %s.%s", column, tablename, reference);          
  
  {
    AdblForeignKeyConstraint* fkconstraint = ENTC_NEW (AdblForeignKeyConstraint);
    
    fkconstraint->name = table->name;
    fkconstraint->column_name = column;
    fkconstraint->table = tablename;
    fkconstraint->reference = reference;
    
    eclist_push_back (table->foreign_keys, fkconstraint);
  }
  
  eclist_destroy (&list);
}

//----------------------------------------------------------------------------------------

AdblTable* adblmodule_parseCreateStatement (const EcString tablename, const EcString statement)
{
  AdblTable* table = adbl_table_new (tablename);
    
  EcString s1 = ecstr_shrink (statement, '(', ')');
  
  EcList list = ectokenizer_parse (s1, ',');

  EcListCursor cursor;
  eclist_cursor_init (list, &cursor, LIST_DIR_NEXT);
  
  while (eclist_cursor_next (&cursor))
  {
    EcString part = eclist_data(cursor.node);
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
  
  eclist_destroy (&list);
  
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
    eclog_msg (LL_WARN, "SQLT", "dbtable", "Not connected to database" );      
    
    return ret;
  }
  
  statement = ecstream_create ();
  
  ecstream_append_str (statement, "SELECT sql FROM sqlite_master WHERE type = 'table' and name='");
  ecstream_append_str (statement, tablename);
  ecstream_append_str (statement, "'");
  
  eclog_msg (LL_TRACE, "SQLT", "dbtable", ecstream_get (statement));
  
  res = sqlite3_prepare_v2 (conn->handle,
                            ecstream_get( statement ),
                            ecstream_size( statement ),
                            &stmt,
                            &p);
  
  // clean up
  ecstream_destroy (&statement);
  
  if (res != SQLITE_OK)
  {
    eclog_msg (LL_ERROR, "SQLT", "dbtable", "Error in last statement");    
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
    eclog_msg (LL_ERROR, "SQLT", "dbtable", "Error in finalize" );
    
    adbl_table_del (&ret);
    
    return 0;  
  }  
  
  return ret;
}

//----------------------------------------------------------------------------------------
