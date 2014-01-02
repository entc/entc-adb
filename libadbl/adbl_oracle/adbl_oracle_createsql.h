#ifndef ADBL_ORACLE_CREATESQL_H
#define ADBL_ORACLE_CREATESQL_H 1

#include "adbl_oracle_common.h"

#include "quomstream.h"

/* forward declarations */
struct QCList;
struct QDBLConstraint;
struct QDBLAttributes;

void qdbl_oracle_createsql_columns( struct QCStream*, struct QCList*, const char* );

void qdbl_oracle_createsql_constraint( struct QCStream*, struct QDBLConstraint* );

void qdbl_oracle_createsql_insert(struct QCStream*, struct QDBLAttributes* );

int qdbl_oracle_createsql_update( struct QCStream*, struct QDBLAttributes* );


#endif
