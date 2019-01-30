#include "adbl.h" 

// cape includes
#include "sys/cape_dl.h"
#include "sys/cape_types.h"

//=============================================================================

typedef void*     (__STDCALL *fct_adbl_pvd_open)          (CapeUdc, CapeErr);
typedef void      (__STDCALL *fct_adbl_pvd_close)         (void**);
typedef int       (__STDCALL *fct_adbl_pvd_begin)         (void*, CapeErr);
typedef int       (__STDCALL *fct_adbl_pvd_commit)        (void*, CapeErr);
typedef int       (__STDCALL *fct_adbl_pvd_rollback)      (void*, CapeErr);

typedef CapeUdc   (__STDCALL *fct_adbl_pvd_get)           (void*, const char* table, CapeUdc* p_params, CapeUdc* p_values, CapeErr);
typedef number_t  (__STDCALL *fct_adbl_pvd_ins)           (void*, const char* table, CapeUdc* p_values, CapeErr);
typedef int       (__STDCALL *fct_adbl_pvd_set)           (void*, const char* table, CapeUdc* p_params, CapeUdc* p_values, CapeErr);
typedef int       (__STDCALL *fct_adbl_pvd_del)           (void*, const char* table, CapeUdc* p_params, CapeErr);
typedef number_t  (__STDCALL *fct_adbl_pvd_ins_or_set)    (void*, const char* table, CapeUdc* p_params, CapeUdc* p_values, CapeErr);

typedef void*     (__STDCALL *fct_adbl_pvd_cursor_new)    (void*, const char* table, CapeUdc* p_params, CapeUdc* p_values, CapeErr);
typedef void      (__STDCALL *fct_adbl_pvd_cursor_del)    (void**);
typedef int       (__STDCALL *fct_adbl_pvd_cursor_next)   (void*);
typedef CapeUdc   (__STDCALL *fct_adbl_pvd_cursor_get)    (void*);

//=============================================================================

typedef struct
{
  fct_adbl_pvd_open           pvd_open;
  fct_adbl_pvd_close          pvd_close;
  fct_adbl_pvd_begin          pvd_begin;
  fct_adbl_pvd_commit         pvd_commit;
  fct_adbl_pvd_rollback       pvd_rollback;
  fct_adbl_pvd_get            pvd_get;
  fct_adbl_pvd_ins            pvd_ins;
  fct_adbl_pvd_set            pvd_set;
  fct_adbl_pvd_del            pvd_del;
  fct_adbl_pvd_ins_or_set     pvd_ins_or_set;
  fct_adbl_pvd_cursor_new     pvd_cursor_new;
  fct_adbl_pvd_cursor_del     pvd_cursor_del;
  fct_adbl_pvd_cursor_next    pvd_cursor_next;
  fct_adbl_pvd_cursor_get     pvd_cursor_get;
  
} AdblPvd;

//=============================================================================

struct AdblCtx_s
{
  CapeDl hlib;  
  
  AdblPvd pvd;
  
};

//-----------------------------------------------------------------------------

AdblCtx adbl_ctx_new (const char* path, const char* backend, CapeErr err)
{
  int res;
  AdblPvd pvd;
  
  CapeDl hlib = cape_dl_new ();

  // try to load the module
  res = cape_dl_load (hlib, path, backend, err);
  if (res)
  {
    goto exit;
  }
  
  pvd.pvd_open = cape_dl_funct (hlib, "adbl_pvd_open", err);
  if (pvd.pvd_open == NULL)
  {
    goto exit;    
  }

  pvd.pvd_close = cape_dl_funct (hlib, "adbl_pvd_close", err);
  if (pvd.pvd_close == NULL)
  {
    goto exit;    
  }

  pvd.pvd_begin = cape_dl_funct (hlib, "adbl_pvd_begin", err);
  if (pvd.pvd_begin == NULL)
  {
    goto exit;    
  }
  
  pvd.pvd_commit = cape_dl_funct (hlib, "adbl_pvd_commit", err);
  if (pvd.pvd_commit == NULL)
  {
    goto exit;    
  }
  
  pvd.pvd_rollback = cape_dl_funct (hlib, "adbl_pvd_rollback", err);
  if (pvd.pvd_rollback == NULL)
  {
    goto exit;    
  }
  
  pvd.pvd_get = cape_dl_funct (hlib, "adbl_pvd_get", err);
  if (pvd.pvd_get == NULL)
  {
    goto exit;    
  }
  
  pvd.pvd_ins = cape_dl_funct (hlib, "adbl_pvd_ins", err);
  if (pvd.pvd_ins == NULL)
  {
    goto exit;    
  }
  
  pvd.pvd_set = cape_dl_funct (hlib, "adbl_pvd_set", err);
  if (pvd.pvd_set == NULL)
  {
    goto exit;    
  }
  
  pvd.pvd_del = cape_dl_funct (hlib, "adbl_pvd_del", err);
  if (pvd.pvd_del == NULL)
  {
    goto exit;    
  }
  
  pvd.pvd_ins_or_set = cape_dl_funct (hlib, "adbl_pvd_ins_or_set", err);
  if (pvd.pvd_ins_or_set == NULL)
  {
    goto exit;    
  }
  
  pvd.pvd_cursor_new = cape_dl_funct (hlib, "adbl_pvd_cursor_new", err);
  if (pvd.pvd_cursor_new == NULL)
  {
    goto exit;    
  }
  
  pvd.pvd_cursor_del = cape_dl_funct (hlib, "adbl_pvd_cursor_del", err);
  if (pvd.pvd_cursor_del == NULL)
  {
    goto exit;    
  }
  
  pvd.pvd_cursor_next = cape_dl_funct (hlib, "adbl_pvd_cursor_next", err);
  if (pvd.pvd_cursor_next == NULL)
  {
    goto exit;    
  }
  
  pvd.pvd_cursor_get = cape_dl_funct (hlib, "adbl_pvd_cursor_get", err);
  if (pvd.pvd_cursor_get == NULL)
  {
    goto exit;    
  }
  
  {
    AdblCtx self = CAPE_NEW(struct AdblCtx_s);
    
    self->hlib = hlib;
    
    memcpy(&(self->pvd), &pvd, sizeof(AdblPvd));
    
    return self;  
  }
  
exit:

  cape_dl_del (&hlib);

  return NULL;
}

//-----------------------------------------------------------------------------

void adbl_ctx_del (AdblCtx* p_self)
{
  AdblCtx self = *p_self;
  
  cape_dl_del (&(self->hlib));
  
  CAPE_DEL(p_self, struct AdblCtx_s);
}

//-----------------------------------------------------------------------------

struct AdblSession_s
{
  const AdblPvd* pvd;
  
  void* session;
};

//=============================================================================

AdblSession adbl_session_open (AdblCtx ctx, CapeUdc connection_properties, CapeErr err)
{
  const AdblPvd* pvd = &(ctx->pvd);
  
  void* session = pvd->pvd_open (connection_properties, err);
  if (session == NULL)
  {
    return NULL;
  }
  
  {
    AdblSession self = CAPE_NEW(struct AdblSession_s);
    
    self->pvd = pvd;
    self->session = session;
    
    return self;
  }
}

//-----------------------------------------------------------------------------

void adbl_session_close (AdblSession* p_self)
{
  AdblSession self = *p_self;
  
  self->pvd->pvd_close (&(self->session));
  
  CAPE_DEL(p_self, struct AdblSession_s);
}

//-----------------------------------------------------------------------------

CapeUdc adbl_session_query (AdblSession self, const char* table, CapeUdc* p_params, CapeUdc* p_values, CapeErr err)
{
  return self->pvd->pvd_get (self->session, table, p_params, p_values, err);
}

//=============================================================================

struct AdblTrx_s
{
  const AdblPvd* pvd;
  
  void* session;
  
  int in_trx;
};

//-----------------------------------------------------------------------------

AdblTrx adbl_trx_new  (AdblSession session, CapeErr err)
{
  AdblTrx self = CAPE_NEW(struct AdblTrx_s);
  
  self->pvd = session->pvd;
  self->session = session->session;

  // don't start with a transaction  
  self->in_trx = FALSE;
  
  return self;
}

//-----------------------------------------------------------------------------

int adbl_trx_commit (AdblTrx* p_self, CapeErr err)
{
  int res = CAPE_ERR_NONE;
  AdblTrx self = *p_self;
  
  if (self->in_trx)
  {
    res = self->pvd->pvd_commit (self->session, err);
  }
    
  CAPE_DEL(p_self, struct AdblTrx_s);
  
  return res;
}

//-----------------------------------------------------------------------------

int adbl_trx_rollback (AdblTrx* p_self, CapeErr err)
{
  int res = CAPE_ERR_NONE;
  AdblTrx self = *p_self;
  
  if (self->in_trx)
  {
    printf ("*** ADBL: ROLLBACK TRANSACTION ***\n");

    res = self->pvd->pvd_rollback (self->session, err);
  }
  
  CAPE_DEL(p_self, struct AdblTrx_s);
  
  return res;
}

//-----------------------------------------------------------------------------

int adbl_trx_start (AdblTrx self, CapeErr err)
{
  if (self->in_trx == FALSE)
  {
    printf ("*** ADBL: START TRANSACTION ***\n");
    
    int res = self->pvd->pvd_begin (self->session, err);
    if (res)
    {
      return res;
    }

    self->in_trx = TRUE;
  }
    
  return CAPE_ERR_NONE;
}

//-----------------------------------------------------------------------------

CapeUdc adbl_trx_query (AdblTrx self, const char* table, CapeUdc* p_params, CapeUdc* p_values, CapeErr err)
{
  int res = adbl_trx_start (self, err);
  if (res)
  {
    return NULL;
  }
  
  return self->pvd->pvd_get (self->session, table, p_params, p_values, err);
}

//-----------------------------------------------------------------------------

number_t adbl_trx_insert (AdblTrx self, const char* table, CapeUdc* p_values, CapeErr err)
{
  int res = adbl_trx_start (self, err);
  if (res)
  {
    return -1;
  }
  
  return self->pvd->pvd_ins (self->session, table, p_values, err);
}

//-----------------------------------------------------------------------------

int adbl_trx_update (AdblTrx self, const char* table, CapeUdc* p_params, CapeUdc* p_values, CapeErr err)
{
  int res = adbl_trx_start (self, err);
  if (res)
  {
    return res;
  }
  
  return self->pvd->pvd_set (self->session, table, p_params, p_values, err);
}

//-----------------------------------------------------------------------------

int adbl_trx_delete (AdblTrx self, const char* table, CapeUdc* p_params, CapeErr err)
{
  int res = adbl_trx_start (self, err);
  if (res)
  {
    return res;
  }
  
  return self->pvd->pvd_del (self->session, table, p_params, err);
}

//-----------------------------------------------------------------------------

number_t adbl_trx_inorup (AdblTrx self, const char* table, CapeUdc* p_params, CapeUdc* p_values, CapeErr err)
{
  int res = adbl_trx_start (self, err);
  if (res)
  {
    return -1;
  }
  
  return self->pvd->pvd_ins_or_set (self->session, table, p_params, p_values, err);
}

//-----------------------------------------------------------------------------

struct AdblCursor_s
{
  const AdblPvd* pvd;
  
  void* handle;
};

//-----------------------------------------------------------------------------

AdblCursor adbl_trx_cursor_new (AdblTrx trx, const char* table, CapeUdc* p_params, CapeUdc* p_values, CapeErr err)
{
  void* handle;
  
  int res = adbl_trx_start (trx, err);
  if (res)
  {
    return NULL;
  }
  
  handle = trx->pvd->pvd_cursor_new (trx->session, table, p_params, p_values, err);

  if (handle == NULL)
  {
    return NULL;
  }

  {
    AdblCursor self = CAPE_NEW(struct AdblCursor_s);
    
    self->pvd = trx->pvd;
    self->handle = handle;
    
    return self;
  }
}

//-----------------------------------------------------------------------------

void adbl_trx_cursor_del (AdblCursor* p_self)
{
  AdblCursor self = *p_self;
  
  self->pvd->pvd_cursor_del (&(self->handle));
  
  CAPE_DEL(p_self, struct AdblCursor_s);
}

//-----------------------------------------------------------------------------

int adbl_trx_cursor_next (AdblCursor self)
{
  return self->pvd->pvd_cursor_next (self->handle);
}

//-----------------------------------------------------------------------------

CapeUdc adbl_trx_cursor_get (AdblCursor self)
{
  return self->pvd->pvd_cursor_get (self->handle);
}

//-----------------------------------------------------------------------------
