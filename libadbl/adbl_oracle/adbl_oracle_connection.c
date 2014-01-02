#include "adbl_oracle_connection.h"

#include "quomutils.h"

/*------------------------------------------------------------------------*/

void
qdbl_oracle_connection_attach(struct QdblOracleConnection* self, const char* database)
{
  /* variables */
  sword status;
  text buffer[401];
  
  /* check system environment */
  const char* tns_admin = getenv("TNS_ADMIN");
  
  if( !tns_admin )
  {
    /* set tns admin for the current directory */
    char buffer[301];
    
    setenv("TNS_ADMIN", getcwd(buffer, 300), 0);
    
    qclogger_logformat(self->logger, LOGMSG_WARNING, "ORAC", "missing TNS_ADMIN env entry, using current directory '%s'", buffer );
  }
  
  if( !self->ocisvr )
  {
    qclogger_log(self->logger, LOGMSG_ERROR, "ORAC", "no enviroment handle" );
    
    return;
  }
  
  status = OCIServerAttach( self->ocisvr, self->ocierr, (const text*)database, strlen(database), OCI_DEFAULT );
  /* handle errors */
  if( qdbl_oracle_error(self->ocierr, status, self->logger, "attach server") )
  {
    return;
  }
  
  /* get the oracle server version */
  status = OCIServerVersion( self->ocisvr, self->ocierr, buffer, 400, OCI_HTYPE_SERVER);
  /* handle errors */
  if( qdbl_oracle_error(self->ocierr, status, self->logger, "get server version") )
  {
    return;
  }
  
  self->version = quomStrCopy((char*)buffer);
  
  qclogger_logformat(self->logger, LOGMSG_DEBUG, "ORAC", "attached to server '%s'", self->version );
  
}

/*------------------------------------------------------------------------*/

void
qdbl_oracle_connection_detach(struct QdblOracleConnection* self)
{
  sword status;
  
  if( !self->ocisvr )
  {
    qclogger_log(self->logger, LOGMSG_ERROR, "ORAC", "{detach} no enviroment handle" );
    
    return;
  }
  
  if( !self->version )
  {
    qclogger_log(self->logger, LOGMSG_ERROR, "ORAC", "{detach} not connected to database" );
    
    return;  
  }
  
  status = OCIServerDetach( self->ocisvr, self->ocierr, OCI_DEFAULT );
  /* handle errors */
  qdbl_oracle_error(self->ocierr, status, self->logger, "detach server");
  
  quomStrDelete(self->version);
  self->version = 0;
}

/*------------------------------------------------------------------------*/

void
qdbl_oracle_connection_allocate_handles(struct QdblOracleConnection* self)
{
  /* variables */
  ub4 mode = OCI_OBJECT | OCI_THREADED; /* 0x00000002 | 0x00000001 */
  sword status;
  
  /* create the env handler */
  status = OCIEnvCreate( &(self->ocienv), mode, 0,0,0,0,0,0 );
  /* error ? */
  if( status != OCI_SUCCESS )
  {
    /* try again */
    status = OCIEnvCreate( &(self->ocienv), mode, 0,0,0,0,0,0 );
    
    if( status != OCI_SUCCESS )
    {
      qclogger_log(self->logger, LOGMSG_ERROR, "ORAC", "can't allocate an oci environment handle" );
      
      self->ocienv = 0;
      
      return;
    }
  }
  /* create the error handler */
  status = OCIHandleAlloc( self->ocienv, (void**)&(self->ocierr), OCI_HTYPE_ERROR, 0, 0 );
  if ( status != OCI_SUCCESS )
  {
    OCIHandleFree( self->ocienv, OCI_HTYPE_ENV );
    
    qclogger_log(self->logger, LOGMSG_ERROR, "ORAC", "can't allocate an oci error handle" );
    
    self->ocienv = 0;
    self->ocierr = 0;
    
    return;    
  }
  /* create the server handler */
  status = OCIHandleAlloc( self->ocienv, (void**)&(self->ocisvr), OCI_HTYPE_SERVER, 0, 0 );
  if ( status != OCI_SUCCESS )
  {
    OCIHandleFree( self->ocierr, OCI_HTYPE_ERROR );
    OCIHandleFree( self->ocienv, OCI_HTYPE_ENV );
    
    qclogger_log(self->logger, LOGMSG_ERROR, "ORAC", "can't allocate an oci server handle" );
    
    self->ocienv = 0;
    self->ocierr = 0;
    self->ocisvr = 0;
    
    return;
  }
}

/*------------------------------------------------------------------------*/

void
qdbl_oracle_connection_release_handles(struct QdblOracleConnection* self)
{
  /* variables */
  sword status;
  
  status = OCIHandleFree( self->ocisvr, OCI_HTYPE_SERVER );
  qdbl_oracle_error(self->ocierr, status, self->logger, "release server handle");
  
  self->ocisvr = 0;
  
  status = OCIHandleFree( self->ocierr, OCI_HTYPE_ERROR );
  if ( status != OCI_SUCCESS )
  {
    qclogger_log(self->logger, LOGMSG_ERROR, "ORAC", "can't release oci error handle" );
  }
  
  self->ocierr = 0;
  
  status = OCIHandleFree( self->ocienv, OCI_HTYPE_ENV );  
  if ( status != OCI_SUCCESS )
  {
    qclogger_log(self->logger, LOGMSG_ERROR, "ORAC", "can't release oci environment handle" );
  }
  
  self->ocienv = 0;
}

/*------------------------------------------------------------------------*/
