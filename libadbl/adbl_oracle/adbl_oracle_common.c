#include "adbl_oracle_common.h"

/*------------------------------------------------------------------------*/

void
qdbl_oracle_error_ocierror(OCIError* ocierr, struct QCLogger* logger, const char* message)
{
  text errbuf[512];
  sb4 errcode;
  
  OCIErrorGet(ocierr, 1, (text *)NULL, &errcode, errbuf, sizeof(errbuf), OCI_HTYPE_ERROR);
  
  qclogger_logformat(logger, LOGMSG_ERROR, "ORAC", "%s : %s", message, errbuf );
}

/*------------------------------------------------------------------------*/

int
qdbl_oracle_error(OCIError* ocierr, sword status, struct QCLogger* logger, const char* message)
{
  switch (status)
  {
    case OCI_SUCCESS:
      return FALSE;
      
    case OCI_SUCCESS_WITH_INFO:
      qdbl_oracle_error_ocierror(ocierr, logger, message);
      return FALSE;
      
    case OCI_NEED_DATA:
      qclogger_logformat(logger, LOGMSG_ERROR, "ORAC", "%s : OCI needs data", message );      
      return TRUE;
      
    case OCI_NO_DATA:
      qclogger_logformat(logger, LOGMSG_ERROR, "ORAC", "%s : OCI found no data", message );      
      return TRUE;
      
    case OCI_ERROR:
      qdbl_oracle_error_ocierror(ocierr, logger, message);
      return TRUE;
      
    case OCI_INVALID_HANDLE:
      qclogger_logformat(logger, LOGMSG_ERROR, "ORAC", "%s : OCI invalid handle", message );      
      return TRUE;
      
    case OCI_STILL_EXECUTING:
      qclogger_logformat(logger, LOGMSG_ERROR, "ORAC", "%s : OCI still executing", message );      
      return TRUE;
      
    default:
      qclogger_logformat(logger, LOGMSG_ERROR, "ORAC", "%s : OCI unknown error", message );      
      return TRUE;
  }
}

/*------------------------------------------------------------------------*/
