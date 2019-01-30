#ifndef __ADBL_MYSQL__H
#define __ADBL_MYSQL__H 1

#include "sys/cape_export.h"
#include "sys/cape_err.h"
#include "stc/cape_udc.h"

//=============================================================================

struct AdblPvdSession_s; typedef struct AdblPvdSession_s* AdblPvdSession;

//-----------------------------------------------------------------------------

__CAPE_LIBEX   AdblPvdSession  __STDCALL adbl_pvd_open              (CapeUdc connection_properties, CapeErr);

__CAPE_LIBEX   void            __STDCALL adbl_pvd_close             (AdblPvdSession*);

//-----------------------------------------------------------------------------

// params    = {"id" : 12, "userid" : 10}           -> ... where id = 12 and userid = 10
// values    = [{"id" : 2, "userid" : 12, "data1" : "hello", "data2" : "world"}]   
//
//    -> SELECT id, userid, data1, data2 FROM ... -> template for return values
//    -> INSERT (id, userid, data1, data2) VALUES (2, 12, "hello", "world")

//-----------------------------------------------------------------------------

__CAPE_LIBEX   CapeUdc         __STDCALL adbl_pvd_get               (AdblPvdSession, const char* table, CapeUdc* p_params, CapeUdc* p_values, CapeErr);    // returns NULL in case of error

__CAPE_LIBEX   number_t        __STDCALL adbl_pvd_ins               (AdblPvdSession, const char* table, CapeUdc* p_values, CapeErr);                       // returns the new ID (ID == 0 -> error)

__CAPE_LIBEX   int             __STDCALL adbl_pvd_del               (AdblPvdSession, const char* table, CapeUdc* p_params, CapeErr);                       // returns error code

__CAPE_LIBEX   int             __STDCALL adbl_pvd_set               (AdblPvdSession, const char* table, CapeUdc* p_params, CapeUdc* p_values, CapeErr);    // returns error code

// don't use
__CAPE_LIBEX   number_t        __STDCALL adbl_pvd_ins_or_set        (AdblPvdSession, const char* table, CapeUdc* p_params, CapeUdc* p_values, CapeErr);    // returns the ID (ID == 0 -> error)

//-----------------------------------------------------------------------------

__CAPE_LIBEX   void            __STDCALL adbl_pvd_auto_commit       (AdblPvdSession, int status);      // turn on/off auto commit (TRUE, FALSE)

__CAPE_LIBEX   int             __STDCALL adbl_pvd_begin             (AdblPvdSession, CapeErr);

__CAPE_LIBEX   int             __STDCALL adbl_pvd_commit            (AdblPvdSession, CapeErr);

__CAPE_LIBEX   int             __STDCALL adbl_pvd_rollback          (AdblPvdSession, CapeErr);

//-----------------------------------------------------------------------------

struct AdblPvdCursor_s; typedef struct AdblPvdCursor_s* AdblPvdCursor;

//-----------------------------------------------------------------------------

__CAPE_LIBEX   AdblPvdCursor   __STDCALL adbl_pvd_cursor_new        (AdblPvdSession, const char* table, CapeUdc* p_params, CapeUdc* p_values, CapeErr);

__CAPE_LIBEX   void            __STDCALL adbl_pvd_cursor_del        (AdblPvdCursor*);

__CAPE_LIBEX   int             __STDCALL adbl_pvd_cursor_next       (AdblPvdCursor);

__CAPE_LIBEX   CapeUdc         __STDCALL adbl_pvd_cursor_get        (AdblPvdCursor);

//=============================================================================

#endif
