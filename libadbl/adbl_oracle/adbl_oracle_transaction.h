#ifndef ADBL_ORACLE_TRANSACTION_H
#define ADBL_ORACLE_TRANSACTION_H 1

#include "adbl_oracle_common.h"

void qdbl_oracle_transaction_start(struct QdblOracleConnection*, struct QCLogger*);

void qdbl_oracle_transaction_commit(struct QdblOracleConnection*, struct QCLogger*);

void qdbl_oracle_transaction_rollback(struct QdblOracleConnection*, struct QCLogger*);

#endif
