/*
 * Copyright (c) 2010-2013 "Alexander Kalkhof" [email:adbl@kalkhof.org]
 *
 * This file is part of adbl framework (Advanced Database Layer)
 *
 * adbl is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * adbl is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with adbl. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef ADBL_ADBL_H
#define ADBL_ADBL_H

#include "adbl_query.h"
#include "adbl_procedure.h"
#include "adbl_constraint.h"
#include "adbl_attributes.h"

#include "adbl_structs.h"
#include "adbl_security.h"
#include "adbl_insert.h"
#include "adbl_update.h"
#include "adbl_delete.h"


typedef struct {
  
  int version;
  
  char sname[5];
  
  const char* name;
  
} AdblModuleInfo;

typedef struct 
{
  
  EcString host;
  
  EcString file;
  
  uint_t port;
  
  EcString schema;
  
  EcString username;
  
  EcString password;
  
} AdblConnectionProperties;

// exported functions definitions

typedef const AdblModuleInfo* (__STDCALL *adbl_info_t)         (void);

typedef void*         (__STDCALL *adbl_dbconnect_t)          (AdblConnectionProperties*);

typedef void          (__STDCALL *adbl_dbdisconnect_t)       (void*);

typedef void*         (__STDCALL *adbl_dbquery_t)            (void*, AdblQuery*);

typedef int           (__STDCALL *adbl_dbprocedure_t)        (void*, AdblProcedure*);

typedef uint_t        (__STDCALL *adbl_dbtable_size_t)       (void*, const EcString);

typedef int           (__STDCALL *adbl_dbupdate_t)           (void*, AdblUpdate*, int);

typedef int           (__STDCALL *adbl_dbinsert_t)           (void*, AdblInsert*);

typedef int           (__STDCALL *adbl_dbdelete_t)           (void*, AdblDelete*);

typedef void          (__STDCALL *adbl_dbbegin_t)            (void*);

typedef void          (__STDCALL *adbl_dbcommit_t)           (void*);

typedef void          (__STDCALL *adbl_dbrollback_t)         (void*);

typedef int           (__STDCALL *adbl_dbcursor_next_t)      (void*);

typedef const char*   (__STDCALL *adbl_dbcursor_data_t)      (void*, uint_t column);

typedef const char*   (__STDCALL *adbl_dbcursor_nextdata_t)  (void*);

typedef void          (__STDCALL *adbl_dbcursor_release_t)   (void*);

typedef void*         (__STDCALL *adbl_dbsequence_get_t)     (void*, const EcString);

typedef void          (__STDCALL *adbl_dbsequence_release_t) (void*);

typedef uint_t        (__STDCALL *adbl_dbsequence_next_t)    (void*);

// addition

typedef EcList        (__STDCALL *adbl_dbschema_t)           (void*);

typedef AdblTable*    (__STDCALL *adbl_dbtable_t)            (void*, const EcString);
                                                             

#ifdef __cplusplus
extern "C" {
#endif 
  
  /* database operations */
  
__ENTC_LIBEX AdblCursor* adbl_dbquery( AdblSession, AdblQuery*, AdblSecurity* );
  
__ENTC_LIBEX int adbl_dbprocedure (AdblSession, AdblProcedure*, AdblSecurity*);

__ENTC_LIBEX uint_t adbl_table_size( AdblSession, const EcString table );
  
  /* sequence operations */
  
__ENTC_LIBEX AdblSequence* adbl_dbsequence_get( AdblSession, const EcString table );
  
__ENTC_LIBEX void adbl_sequence_release( AdblSequence** );
  
__ENTC_LIBEX uint_t adbl_sequence_next( AdblSequence* );
  
  /* cursor operations */
  
__ENTC_LIBEX int adbl_dbcursor_next( AdblCursor* );
  
__ENTC_LIBEX const char* adbl_dbcursor_data( AdblCursor*, uint_t column );
  
__ENTC_LIBEX const char* adbl_dbcursor_nextdata( AdblCursor* );
  
__ENTC_LIBEX void adbl_dbcursor_release( AdblCursor** );
  
__ENTC_LIBEX int adbl_dbupdate( AdblSession, AdblUpdate*, int isInsert, AdblSecurity* );
  
__ENTC_LIBEX int adbl_dbinsert( AdblSession, AdblInsert*, AdblSecurity* );
  
__ENTC_LIBEX int adbl_dbdelete( AdblSession, AdblDelete*, AdblSecurity* );
  
__ENTC_LIBEX void adbl_dbbegin( AdblSession );
  
__ENTC_LIBEX void adbl_dbcommit( AdblSession );
  
__ENTC_LIBEX void adbl_dbrollback( AdblSession );
  
__ENTC_LIBEX EcList adbl_dbschema (AdblSession); 
  
__ENTC_LIBEX AdblTable* adbl_dbtable (AdblSession, const EcString);  
  
#ifdef __cplusplus
}
#endif


#endif

