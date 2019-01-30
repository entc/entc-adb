#ifndef __ADBL_MYSQL__PREPARE_H
#define __ADBL_MYSQL__PREPARE_H 1

#include "bindvars.h"
#include "adbl_mysql.h"

//-----------------------------------------------------------------------------

// mysql includes
#include <mysql.h>

// cape includes
#include "sys/cape_export.h"
#include "sys/cape_err.h"
#include "stc/cape_udc.h"

//=============================================================================

struct AdblPrepare_s; typedef struct AdblPrepare_s* AdblPrepare;

//-----------------------------------------------------------------------------

struct AdblPvdCursor_s
{
  MYSQL_STMT* stmt;
  
  number_t pos;
  
  AdblBindVars binds;
  
  CapeUdc values;
  
};

//-----------------------------------------------------------------------------

__CAPE_LIBEX   AdblPrepare     adbl_prepare_new                (MYSQL* mysql, CapeUdc* p_params, CapeUdc* p_values);

__CAPE_LIBEX   void            adbl_prepare_del                (AdblPrepare*);

__CAPE_LIBEX   AdblPvdCursor   adbl_prepare_to_cursor          (AdblPrepare*);

//-----------------------------------------------------------------------------

__CAPE_LIBEX   int             adbl_prepare_binds_params       (AdblPrepare, CapeErr err);

__CAPE_LIBEX   int             adbl_prepare_binds_result       (AdblPrepare, CapeErr err);

__CAPE_LIBEX   int             adbl_prepare_binds_values       (AdblPrepare, CapeErr err);

__CAPE_LIBEX   int             adbl_prepare_binds_all          (AdblPrepare, CapeErr err);

__CAPE_LIBEX   number_t        adbl_prepare_execute            (AdblPrepare, MYSQL* mysql, CapeErr err);

//-----------------------------------------------------------------------------

__CAPE_LIBEX   int             adbl_prepare_statement_select   (AdblPrepare, const char* schema, const char* table, int ansi, CapeErr err);

__CAPE_LIBEX   int             adbl_prepare_statement_insert   (AdblPrepare, const char* schema, const char* table, int ansi, CapeErr err);

__CAPE_LIBEX   int             adbl_prepare_statement_delete   (AdblPrepare, const char* schema, const char* table, int ansi, CapeErr err);

__CAPE_LIBEX   int             adbl_prepare_statement_update   (AdblPrepare, const char* schema, const char* table, int ansi, CapeErr err);

__CAPE_LIBEX   int             adbl_prepare_statement_setins   (AdblPrepare, const char* schema, const char* table, int ansi, CapeErr err);

//-----------------------------------------------------------------------------

#endif
