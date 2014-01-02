#include "adbl_oracle_common.h"
#include "adbl_oracle_connection.h"
#include "adbl_oracle_transaction.h"
#include "adbl_oracle_createsql.h"


#include "adbl_matrix.h"
#include "quomsys.h"

#include "quomutils.h"

#include "quomlist.h"

#include "adbl_structs.h"

/*------------------------------------------------------------------------*/

struct QdblOracleOutputVariable
{
  
  char* value;
  
  sb2 indicators;
  
};

/*------------------------------------------------------------------------*/

void
qdbl_oracle_session_start(struct QdblOracleConnection* self, const char* username, const char* password)
{
  /* variables */
  sword status;
  
  /* create a service handle */
  status = OCIHandleAlloc(self->ocienv, (void**)&(self->ocicontext), OCI_HTYPE_SVCCTX, 0, 0);

  if( qdbl_oracle_error(self->ocierr, status, self->logger, "create svcctx") )
  {
    /* set to connect */
    self->ocicontext = 0;
    
    return;
  }
  
  /* setting the server to the service context */
  status = OCIAttrSet(self->ocicontext, OCI_HTYPE_SVCCTX, self->ocisvr, 0, OCI_ATTR_SERVER, self->ocierr);

  if( qdbl_oracle_error(self->ocierr, status, self->logger, "set svcctx") )
  {
    OCIHandleFree(self->ocicontext, OCI_HTYPE_SVCCTX);
    /* set to connect */
    self->ocicontext = 0;
    
    return;
  }
  
  /* create session handle */
  status = OCIHandleAlloc(self->ocienv, (void**)&(self->ocisession), OCI_HTYPE_SESSION, 0, 0);
  
  if( qdbl_oracle_error(self->ocierr, status, self->logger, "create session") )
  {
    OCIHandleFree(self->ocicontext, OCI_HTYPE_SVCCTX);
    /* set to connect */
    self->ocisession = 0;
    self->ocicontext = 0;
    
    return;
  }
  
  /* setting the session to the service context */
  status = OCIAttrSet( self->ocicontext, OCI_HTYPE_SVCCTX, self->ocisession, 0, OCI_ATTR_SESSION, self->ocierr );

  if( qdbl_oracle_error(self->ocierr, status, self->logger, "set session") )
  {
    OCIHandleFree(self->ocisession, OCI_HTYPE_SESSION);
    OCIHandleFree(self->ocicontext, OCI_HTYPE_SVCCTX);
    /* set to connect */
    self->ocisession = 0;
    self->ocicontext = 0;
    
    return;
  }
  
  /* setting username and password */
  status = OCIAttrSet(self->ocisession, OCI_HTYPE_SESSION, (char*)username, strlen(username), OCI_ATTR_USERNAME, self->ocierr);

  if( qdbl_oracle_error(self->ocierr, status, self->logger, "set username") )
  {
    OCIHandleFree(self->ocisession, OCI_HTYPE_SESSION);
    OCIHandleFree(self->ocicontext, OCI_HTYPE_SVCCTX);
    /* set to connect */
    self->ocisession = 0;
    self->ocicontext = 0;
    
    return;
  }

  status = OCIAttrSet(self->ocisession, OCI_HTYPE_SESSION, (char*)password, strlen(password), OCI_ATTR_PASSWORD, self->ocierr);
  
  if( qdbl_oracle_error(self->ocierr, status, self->logger, "set password") )
  {
    OCIHandleFree(self->ocisession, OCI_HTYPE_SESSION);
    OCIHandleFree(self->ocicontext, OCI_HTYPE_SVCCTX);
    /* set to connect */
    self->ocisession = 0;
    self->ocicontext = 0;
    
    return;
  }
  
  /* begin session and authenticating */

  status = OCISessionBegin(self->ocicontext, self->ocierr, self->ocisession, OCI_CRED_RDBMS, OCI_DEFAULT);

  if( qdbl_oracle_error(self->ocierr, status, self->logger, "begin session") )
  {
    OCIHandleFree(self->ocisession, OCI_HTYPE_SESSION);
    OCIHandleFree(self->ocicontext, OCI_HTYPE_SVCCTX);
    /* set to connect */
    self->ocisession = 0;
    self->ocicontext = 0;
    
    return;
  }
  
  /* success */
  qclogger_log(self->logger, LOGMSG_SUCCESS, "ORAC", "new session started" );
  
}

/*------------------------------------------------------------------------*/

void
qdbl_oracle_session_stop(struct QdblOracleConnection* self)
{
  /* variables */
  sword status;
  
  status = OCISessionEnd(self->ocicontext, self->ocierr, self->ocisession, OCI_DEFAULT);
  
  qdbl_oracle_error(self->ocierr, status, self->logger, "end session");
  
  /* release resources */
  OCIHandleFree(self->ocisession, OCI_HTYPE_SESSION);
  self->ocisession = 0;
  OCIHandleFree(self->ocicontext, OCI_HTYPE_SVCCTX);
  self->ocicontext = 0;
}

/*------------------------------------------------------------------------*/

void*
qdbl_oracle_dbconnect(struct QDBLConnectionProperties* cp, struct QCLogger* logger )
{
  struct QdblOracleConnection* conn = QNEW(struct QdblOracleConnection);
  /* init */
  conn->logger = logger;
  conn->ocienv = 0;
  conn->ocierr = 0;
  conn->ocisvr = 0;
  conn->version = 0;
  conn->ocicontext = 0;
  conn->ocisession = 0;
  conn->ocitrans = 0;
  
  qdbl_oracle_connection_allocate_handles(conn);
  
  qdbl_oracle_connection_attach(conn, cp->host);
  
  qdbl_oracle_session_start(conn, cp->username, cp->password);
  
  return conn;
}

/*------------------------------------------------------------------------*/

void
qdbl_oracle_dbdisconnect( void* ptr, struct QCLogger* logger )
{
  struct QdblOracleConnection* conn = ptr;
  
  qdbl_oracle_session_stop(conn);
  
  qdbl_oracle_connection_detach(conn);
  
  qdbl_oracle_connection_release_handles(conn);

  free(conn);
}

/*------------------------------------------------------------------------*/

OCIStmt*
qdbl_oracle_statement_prepare(struct QdblOracleConnection* self, const char* statement, qlong_t size, struct QCLogger* logger)
{
  /* variables */
  sword status;
  OCIStmt* ocistmt;
  
  /* create the statement */
  status = OCIStmtPrepare2( self->ocicontext, &ocistmt, self->ocierr, (const text*)statement, size, 0, 0, OCI_NTV_SYNTAX, OCI_DEFAULT );
  
  if( qdbl_oracle_error(self->ocierr, status, self->logger, "prepare statement") )
  {
    return 0;
  }
  
  return ocistmt;
}

/*------------------------------------------------------------------------*/

void
qdbl_oracle_statement_define(struct QdblOracleConnection* self, OCIStmt* ocistmt, const struct QCList* columns, struct QCList* values)
{
  /* variables */
  sword status;
  struct QCListNode* node;
  qlong_t position = 1;
  
  for( node = qclist_first(columns); node != qclist_end(columns); node = qclist_next(node) )
  {
    struct QdblOracleOutputVariable* ov = QNEW(struct QdblOracleOutputVariable);
    
    ov->indicators = 0;
    ov->value = (char*)malloc( sizeof(char) * (401) );
    
    qclist_append( values, ov );
    
    OCIDefine* definePointer = 0;
    
    OCIParam* parameter = 0;

    ub2 dataType = 0;

    /* get the parameter of the column */
    status = OCIParamGet( ocistmt, OCI_HTYPE_STMT, self->ocierr, (void**)&parameter, position );

    qdbl_oracle_error(self->ocierr, status, self->logger, "get column parameter");

    /* get the type of the column */
    /*
    status = OCIAttrGet( parameter, OCI_DTYPE_PARAM,
                         &dataType, 0, OCI_ATTR_DATA_TYPE,
                         self->ocierr );
    */    
    //qdbl_oracle_error(self->ocierr, status, self->logger, "get column type");
    
    
    status = OCIDefineByPos( ocistmt, &definePointer, self->ocierr,
                                  position,
                                  &(ov->value),
                                  400,
                                  SQLT_CHR,
                                  &(ov->indicators),
                                  0,
                                  0,
                                  OCI_DEFAULT );
    
    qdbl_oracle_error(self->ocierr, status, self->logger, "define output");
    
    position++;
  }
  
  
}

/*------------------------------------------------------------------------*/

qlong_t
qdbl_oracle_statement_execute(struct QdblOracleConnection* self, OCIStmt* ocistmt, qlong_t count)
{
  /* variables */
  sword status;
  ub4 result = 0;
  
  /* execute query */
  status = OCIStmtExecute( self->ocicontext, ocistmt, self->ocierr, count, 0, 0, 0, OCI_DEFAULT);
  
  if( qdbl_oracle_error(self->ocierr, status, self->logger, "execute statement") )
  {
    return 0;
  }  
  
  /* get the processed rows */
  status = OCIAttrGet( ocistmt, OCI_HTYPE_STMT, &result, 0, OCI_ATTR_ROW_COUNT, self->ocierr );

  qdbl_oracle_error(self->ocierr, status, self->logger, "get processed rows");
  
  return result;
}

/*------------------------------------------------------------------------*/

struct QdblOracleCursor*
qdbl_oracle_statement_query(struct QdblOracleConnection* self, struct QDBLQuery* query, struct QCLogger* logger)
{
  struct QdblOracleCursor* cursor = 0;
  OCIStmt* ocistmt;
  /* create the stream */
  struct QCStream* statement = qcstream_new();
  struct QCList* values = qclist_new();
  
  /* construct the stream */
  qcstream_append( statement, "SELECT " );

  qdbl_oracle_createsql_columns( statement, query->columns, query->table );
  
  qcstream_append( statement, " FROM " );
  qcstream_append( statement, query->table );
  
  if(query->constraint)
  {
    qdbl_oracle_createsql_constraint( statement, query->constraint );
  }
  
  qclogger_log(logger, LOGMSG_SQL, "ORAC", qcstream_buffer( statement ) );
  
  ocistmt = qdbl_oracle_statement_prepare(self, qcstream_buffer( statement ), qcstream_size( statement ), logger);

  /* clean first the stream */
  qcstream_delete( statement );
  

  /* execute */
  if( qdbl_oracle_statement_execute(self, ocistmt, 1) > 0 )
  {
    cursor = QNEW(struct QdblOracleCursor);
    /* init */
    cursor->values = values;
    cursor->logger = self->logger;
    cursor->ocierr = self->ocierr;
    cursor->ocistmt = ocistmt;
    cursor->pos = 0;    
    
    /* define first all output variables */
    qdbl_oracle_statement_define(self, ocistmt, query->columns, values);

  }
  else
  {
    qclist_delete( values );  
  }
  
  return cursor;
}

/*------------------------------------------------------------------------*/

void
qdbl_oracle_statement_release(struct QdblOracleConnection* self, OCIStmt* ocistmt)
{
  /* variables */
  sword status;
  
  /* release the statement handle */
  status = OCIStmtRelease( ocistmt, self->ocierr, 0, 0, OCI_DEFAULT );
  
  qdbl_oracle_error(self->ocierr, status, self->logger, "release cursor");
}
  
/*------------------------------------------------------------------------*/

void*
qdbl_oracle_dbquery( void* ptr, struct QDBLQuery* query, struct QCLogger* logger )
{
  struct QdblOracleConnection* self = ptr;
  struct QdblOracleCursor* cursor = 0;
  
  /* do we have a transaction started */
  /*
  if( !self->ocitrans )
  {
    qclogger_log(self->logger, LOGMSG_ERROR, "ORAC", "no active transaction" );
    
    return 0;
  }
  */
  cursor = qdbl_oracle_statement_query(self, query, logger);
  
  return cursor;
}

/*------------------------------------------------------------------------*/

qlong_t
qdbl_oracle_table_size( void* ptr, const char* table, struct QCLogger* logger )
{
  struct QdblOracleConnection* self = ptr;
  /* variables */
  sword status;
  OCIStmt* ocistmt;
  qlong_t value = 0;
  struct QCStream* statement;
  
  /* do we have a transaction started */
  
  /*
  if( !self->ocitrans )
  {
    qclogger_log(self->logger, LOGMSG_ERROR, "ORAC", "no active transaction" );
    
    return 0;
  }
  */
  
  /* create the stream */
  statement = qcstream_new();
  /* construct the stream */
  qcstream_append( statement, "SELECT COUNT(*) FROM " );
  qcstream_append( statement, table );
  
  qclogger_log(logger, LOGMSG_SQL, "ORAC", qcstream_buffer( statement ) );
  
  /* prepare the statement */
  ocistmt = qdbl_oracle_statement_prepare(self, qcstream_buffer( statement ), qcstream_size( statement ), logger);
  
  if( ocistmt )
  {
    OCIDefine* definePointer = 0;
    
    status = OCIDefineByPos( ocistmt, &definePointer, self->ocierr,
                            1,
                            &(value),
                            sizeof(qlong_t),
                            SQLT_UIN,
                            0,
                            0,
                            0,
                            OCI_DEFAULT );
    
    qdbl_oracle_error(self->ocierr, status, logger, "define sequence");
    
    /* execute */
    qdbl_oracle_statement_execute(self, ocistmt, 1);

    qdbl_oracle_statement_release(self, ocistmt);
  }
  
  /* release stream */
  qcstream_delete( statement );

  return value;
}

/*------------------------------------------------------------------------*/

int
qdbl_oracle_dbupdate( void* ptr, struct QDBLUpdate* update, struct QCLogger* logger )
{
  struct QdblOracleConnection* self = ptr;
  int res = 0;
  /* variables */
  OCIStmt* ocistmt;
  /* create the stream */
  struct QCStream* statement = qcstream_new();

  /* do we have a transaction started */
  if( !self->ocitrans )
  {
    qclogger_log(self->logger, LOGMSG_ERROR, "ORAC", "no active transaction" );
    
    return 0;
  }
  /* construct the stream */
  qcstream_append( statement, "UPDATE " );
  qcstream_append( statement, update->table );
  qcstream_append( statement, " SET " );
 
  if( !qdbl_oracle_createsql_update(statement, update->attrs) )
  {
    qcstream_delete( statement );
    
    return 0;
  }
  
  if(update->constraint)
  {
    qdbl_oracle_createsql_constraint( statement, update->constraint );
  }

  qclogger_log(logger, LOGMSG_SQL, "ORAC", qcstream_buffer( statement ) );
  
  /* prepare the statement */
  ocistmt = qdbl_oracle_statement_prepare(self, qcstream_buffer( statement ), qcstream_size( statement ), logger);

  if( ocistmt )
  {
    /* execute */
    res = qdbl_oracle_statement_execute(self, ocistmt, 1);
    
    qdbl_oracle_statement_release(self, ocistmt);
    
  }
  
  return res;
}

/*------------------------------------------------------------------------*/

int
qdbl_oracle_dbinsert( void* ptr, struct QDBLInsert* insert, struct QCLogger* logger )
{
  struct QdblOracleConnection* self = ptr;
  int res = 0;
  /* variables */
  OCIStmt* ocistmt;
  /* create the stream */
  struct QCStream* statement = qcstream_new();
  
  /* do we have a transaction started */
  if( !self->ocitrans )
  {
    qclogger_log(self->logger, LOGMSG_ERROR, "ORAC", "no active transaction" );
    
    return 0;
  }  
  /* construct the stream */
  qcstream_append( statement, "INSERT INTO " );
  qcstream_append( statement, insert->table );
  
  qdbl_oracle_createsql_insert(statement, insert->attrs );

  qclogger_log(logger, LOGMSG_SQL, "ORAC", qcstream_buffer( statement ) );
  
  /* prepare the statement */
  ocistmt = qdbl_oracle_statement_prepare(self, qcstream_buffer( statement ), qcstream_size( statement ), logger);
  
  if( ocistmt )
  {
    /* execute */
    res = qdbl_oracle_statement_execute(self, ocistmt, 1);
  
    qdbl_oracle_statement_release(self, ocistmt);
  
  }
    
  return res;
}

/*------------------------------------------------------------------------*/

int
qdbl_oracle_dbdelete( void* ptr, struct QDBLDelete* del, struct QCLogger* logger )
{

  return 0;
}

/*------------------------------------------------------------------------*/

void
qdbl_oracle_dbbegin( void* ptr, struct QCLogger* logger )
{  
  qdbl_oracle_transaction_start(ptr, logger);
}

/*------------------------------------------------------------------------*/

void
qdbl_oracle_dbcommit( void* ptr, struct QCLogger* logger )
{
  qdbl_oracle_transaction_commit(ptr, logger);
}

/*------------------------------------------------------------------------*/

void
qdbl_oracle_dbrollback( void* ptr, struct QCLogger* logger )
{
  qdbl_oracle_transaction_rollback(ptr, logger);
}

/*------------------------------------------------------------------------*/

int
qdbl_oracle_dbcursor_next( void* ptr )
{
  struct QdblOracleCursor* self = ptr;
  
  /* variables */
  sword status;
  
  status = OCIStmtFetch2( self->ocistmt, self->ocierr, 1, OCI_FETCH_NEXT, 0, OCI_DEFAULT );

  if ( status == OCI_NO_DATA )
  {
    qclogger_log(self->logger, LOGMSG_DEBUG, "ORAC", "retrieved OCI_NO_DATA" );
    
    return FALSE;  
  }
  
  if( qdbl_oracle_error(self->ocierr, status, self->logger, "next cursor") )
  {
    return FALSE;
  }

  return TRUE;
}

/*------------------------------------------------------------------------*/

const char*
qdbl_oracle_dbcursor_data( void* ptr, qlong_t column )
{
  
  return 0;
}

/*------------------------------------------------------------------------*/

const char*
qdbl_oracle_dbcursor_nextdata( void* ptr )
{

  return 0;
}

/*------------------------------------------------------------------------*/

void
qdbl_oracle_dbcursor_release( void* ptr )
{
  struct QdblOracleCursor* self = ptr;

  /* variables */
  sword status;

  /* release the statement handle */
  status = OCIStmtRelease( self->ocistmt, self->ocierr, 0, 0, OCI_DEFAULT );
  
  qdbl_oracle_error(self->ocierr, status, self->logger, "release cursor");
}

/*------------------------------------------------------------------------*/

void*
qdbl_oracle_dbsequence_get( void* ptr, const char* table, struct QCLogger* logger )
{
  struct QdblOracleConnection* conn = ptr;
  struct QdblOracleSequence* self = QNEW(struct QdblOracleSequence);
  /* create the stream */
  struct QCStream* statement = qcstream_new();
  /* variables */
  sword status;

  /* init */
  self->conn = conn;
  self->ocistmt = 0;
  
  /* construct the stream */
  qcstream_append( statement, "SELECT seq_" );
  qcstream_append( statement, table );
  qcstream_append( statement, ".NEXTVAL FROM DUAL" );
  
  qclogger_log(logger, LOGMSG_SQL, "ORAC", qcstream_buffer( statement ) );
  
  /* prepare the statement */
  self->ocistmt = qdbl_oracle_statement_prepare(conn, qcstream_buffer( statement ), qcstream_size( statement ), logger);
  
  if( self->ocistmt )
  {
    OCIDefine* definePointer = 0;
    
    status = OCIDefineByPos( self->ocistmt, &definePointer, conn->ocierr,
                            1,
                            &(self->value),
                            sizeof(qlong_t),
                            SQLT_UIN,
                            0,
                            0,
                            0,
                            OCI_DEFAULT );
    
    qdbl_oracle_error(conn->ocierr, status, conn->logger, "define sequence");
  }
  
  /* release stream */
  qcstream_delete( statement );
  
  return self;
}

/*------------------------------------------------------------------------*/

void
qdbl_oracle_dbsequence_release( void* ptr, struct QCLogger* logger )
{
  struct QdblOracleSequence* self = ptr;

  qdbl_oracle_statement_release(self->conn, self->ocistmt);
  
  free( self );
}

/*------------------------------------------------------------------------*/

qlong_t
qdbl_oracle_dbsequence_next( void* ptr, struct QCLogger* logger )
{
  struct QdblOracleSequence* self = ptr;
  
  /* reset the value */
  self->value = 0;
  
  if( self->ocistmt )
  {
    /* execute */
    qdbl_oracle_statement_execute(self->conn, self->ocistmt, 1);
  }
  return self->value;
}

/*------------------------------------------------------------------------*/

char*
adblPlugin()
{
  return "Oracle";
}

/*------------------------------------------------------------------------*/

static struct QDBLExeMatrix oracle_matrix = 
{
  qdbl_oracle_dbconnect,
  qdbl_oracle_dbdisconnect,
  qdbl_oracle_dbquery,
  qdbl_oracle_table_size,
  qdbl_oracle_dbupdate,
  qdbl_oracle_dbinsert,
  qdbl_oracle_dbdelete,
  qdbl_oracle_dbbegin,
  qdbl_oracle_dbcommit,
  qdbl_oracle_dbrollback,
  qdbl_oracle_dbcursor_next,
  qdbl_oracle_dbcursor_data,
  qdbl_oracle_dbcursor_nextdata,
  qdbl_oracle_dbcursor_release,
  qdbl_oracle_dbsequence_get,
  qdbl_oracle_dbsequence_release,
  qdbl_oracle_dbsequence_next
};

/*------------------------------------------------------------------------*/

struct QDBLExeMatrix*
getQDBLExeMatrix()
{
  return &oracle_matrix;
}

/*------------------------------------------------------------------------*/
