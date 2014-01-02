#ifndef ADBL_ORACLE_COMMON_H
#define ADBL_ORACLE_COMMON_H 1

/* quom includes */
#include "quomsys.h"
#include "quomutex.h"
#include "quomlogger.h"

#include <oci.h>

/* definition */
struct QdblOracleConnection
{
  
  char* schema;
  
  qmutex_t* mutex;
  
  struct QCLogger* logger;
  
  /* oracle handlers */
  OCIEnv* ocienv;
  
  OCIError* ocierr;
  
  OCIServer* ocisvr;
  
  char* version;
  
  /* oracle session handles */
  OCISvcCtx* ocicontext;
  
  OCISession* ocisession;
  
  /* oracle transaction handles */
  OCITrans* ocitrans;
  
};

struct QdblOracleCursor
{
  
  /* list of char* owned */
  struct QCList* values;
  
  qlong_t pos;
  
  /* references */
  OCIError* ocierr;
  
  struct QCLogger* logger;
  
  /* statement handle */
  OCIStmt* ocistmt;
  
};

struct QdblOracleSequence
{
  /* references */
  struct QdblOracleConnection* conn;
  
  /* statement handle */
  OCIStmt* ocistmt;
  
  qlong_t value;
    
};

int qdbl_oracle_error(OCIError*, sword, struct QCLogger*, const char*);

#endif
