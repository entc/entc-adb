#include "adbo2.h"

// adbl includes
#include <adbl.h>
#include <adbl_structs.h>
#include <adbl_manager.h>

//-----------------------------------------------------------------------------

struct Adbo2Transaction_s
{
  AdblSession session;
  
};

//-----------------------------------------------------------------------------

Adbo2Transaction adbo2_trx_create (struct AdblManager_s* adblm)
{
  Adbo2Transaction self = ENTC_NEW (struct Adbo2Transaction_s);
  
  self->session = adbl_openSession (adblm, "default");
  
  // start transaction in the database
  adbl_dbbegin (self->session);
  
  return self;
}

//-----------------------------------------------------------------------------

void adbo2_trx_destroy (Adbo2Transaction* pself)
{
  Adbo2Transaction self = *pself;
  
  adbl_closeSession (&(self->session));
  
  ENTC_DEL (pself, struct Adbo2Transaction_s);
}

//-----------------------------------------------------------------------------

void adbo2_trx_commit (Adbo2Transaction* pself)
{
  Adbo2Transaction self = *pself;
  
  // commit transaction in the database
  adbl_dbcommit (self->session);
  
  adbo2_trx_destroy (pself);
}

//-----------------------------------------------------------------------------

void adbo2_trx_rollback (Adbo2Transaction* pself)
{
  Adbo2Transaction self = *pself;

  // rollback transaction in the database
  adbl_dbrollback (self->session);

  adbo2_trx_destroy (pself);
}

//-----------------------------------------------------------------------------

int adbo2_trx_query (Adbo2Transaction self, const EcString table, EcUdc params, EcUdc data, EcErr err)
{
  
}

//-----------------------------------------------------------------------------

int adbo2_trx_update (Adbo2Transaction self, const EcString table, EcUdc params, EcUdc data, EcErr err)
{ 
  
}

//-----------------------------------------------------------------------------

int adbo2_trx_insert (Adbo2Transaction self, const EcString table, EcUdc params, EcUdc data, EcErr err)
{
  
}

//-----------------------------------------------------------------------------

int adbo2_trx_delete (Adbo2Transaction self, const EcString table, EcUdc params, EcErr err)
{
  int res;
  AdblSecurity adblsec;      

  // create a constraint
  AdblConstraint* constraint = adbl_constraint_new (QUOMADBL_CONSTRAINT_AND);
  
  // create the delete context
  AdblDelete* deleteCtx = adbl_delete_new ();

  // set constraints as parameters
  adbl_delete_setConstraint (deleteCtx, constraint);
  
  // set table name
  adbl_delete_setTable (deleteCtx, table);
  
  // execute on database
  res = adbl_dbdelete (self->session, deleteCtx, &adblsec);
      
  // cleanup
  adbl_delete_delete(&deleteCtx);
  adbl_constraint_delete(&constraint);
  
  if (res)
  {
    return ENTC_ERR_NONE;
  }
  else
  {
    
  }
}

//-----------------------------------------------------------------------------

struct Adbo2_s
{
  
  struct AdblManager_s* adblm;
  
};

//-----------------------------------------------------------------------------

Adbo2 adbo2_create (const EcString confPath, const EcString binPath)
{
  Adbo2 self = ENTC_NEW (struct Adbo2_s);
  
  
  
  
  return self;
}

//-----------------------------------------------------------------------------

void adbo2_destroy (Adbo2* pself)
{
  
}

//-----------------------------------------------------------------------------

int adbo2_init (Adbo2 self, const EcString jsonConf)
{
  
}

//-----------------------------------------------------------------------------

Adbo2Transaction adbo2_transaction (Adbo2 self)
{
  return adbo2_trx_create (self->adblm);
}

//-----------------------------------------------------------------------------

