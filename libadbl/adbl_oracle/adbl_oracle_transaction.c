#include "adbl_oracle_transaction.h"

/*------------------------------------------------------------------------*/

void
qdbl_oracle_transaction_release(struct QdblOracleConnection* self)
{
  /* variables */
  sword status;
  
  status = OCIHandleFree(self->ocitrans, OCI_HTYPE_TRANS);
  
  qdbl_oracle_error(self->ocierr, status, self->logger, "delete transaction");
  
  /* clear the transaction attribute in the service context */
  status = OCIAttrSet(self->ocicontext, OCI_HTYPE_SVCCTX, 0, 0, OCI_ATTR_TRANS, self->ocierr);
  
  qdbl_oracle_error(self->ocierr, status, self->logger, "release transaction");
  
  self->ocitrans = 0;
}

/*------------------------------------------------------------------------*/

void
qdbl_oracle_transaction_start(struct QdblOracleConnection* self, struct QCLogger* logger)
{
  /* variables */
  sword status;
  
  /* allocate oci handles */
  status = OCIHandleAlloc(self->ocienv, (void**)&(self->ocitrans), OCI_HTYPE_TRANS, 0, 0);
  
  if( qdbl_oracle_error(self->ocierr, status, self->logger, "create transaction") )
  {
    self->ocitrans = 0;
    
    return;
  }
  
  /* set transaction attribute in the service context */
  status = OCIAttrSet(self->ocicontext, OCI_HTYPE_SVCCTX, self->ocitrans, 0, OCI_ATTR_TRANS, self->ocierr);
  
  if( qdbl_oracle_error(self->ocierr, status, self->logger, "set transaction") )
  {
    OCIHandleFree(self->ocitrans, OCI_HTYPE_TRANS);
    self->ocitrans = 0;
    
    return;
  }
  
  /* start transaction */
  status = OCITransStart(self->ocicontext, self->ocierr, 0, OCI_TRANS_NEW);
  
  if( qdbl_oracle_error(self->ocierr, status, self->logger, "start transaction") )
  {
    qdbl_oracle_transaction_release(self);
    
    return;
  }
  
  qclogger_log(self->logger, LOGMSG_DEBUG, "ORAC", "transaction started" );
}

/*------------------------------------------------------------------------*/

void
qdbl_oracle_transaction_commit(struct QdblOracleConnection* self, struct QCLogger* logger)
{
  /* variables */
  sword status;
  
  if( !self->ocitrans )
  {
    qclogger_log(self->logger, LOGMSG_ERROR, "ORAC", "{commit} no valid transaction" );
    
    return;
  }
  
  /* commit */
  status = OCITransCommit(self->ocicontext, self->ocierr, OCI_DEFAULT);
  
  if( !qdbl_oracle_error(self->ocierr, status, self->logger, "commit transaction") )
  {
    qclogger_log(self->logger, LOGMSG_DEBUG, "ORAC", "transaction committed" );
  }
  
  qdbl_oracle_transaction_release(self);
}

/*------------------------------------------------------------------------*/

void
qdbl_oracle_transaction_rollback(struct QdblOracleConnection* self, struct QCLogger* logger)
{
  /* variables */
  sword status;
  
  if( !self->ocitrans )
  {
    qclogger_log(self->logger, LOGMSG_ERROR, "ORAC", "{rollback} no valid transaction" );
    
    return;
  }
  
  /* commit */
  status = OCITransRollback(self->ocicontext, self->ocierr, OCI_DEFAULT);
  
  if( !qdbl_oracle_error(self->ocierr, status, self->logger, "rollback transaction") )
  {
    qclogger_log(self->logger, LOGMSG_DEBUG, "ORAC", "transaction rolled back" );
  }
  
  qdbl_oracle_transaction_release(self);
}

/*------------------------------------------------------------------------*/
