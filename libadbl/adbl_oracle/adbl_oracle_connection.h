#ifndef ADBL_ORACLE_CONNECTION_H
#define ADBL_ORACLE_CONNECTION_H 1

#include "adbl_oracle_common.h"

void qdbl_oracle_connection_attach(struct QdblOracleConnection*, const char*);

void qdbl_oracle_connection_detach(struct QdblOracleConnection*);

void qdbl_oracle_connection_allocate_handles(struct QdblOracleConnection*);

void qdbl_oracle_connection_release_handles(struct QdblOracleConnection*);

#endif
