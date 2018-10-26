#include "adbo2.h"

// entc includes
#include <tools/ecjson.h>
#include <tools/eclog.h>

// adbl includes
#include <adbl.h>
#include <adbl_structs.h>
#include <adbl_manager.h>

//-----------------------------------------------------------------------------

struct Adbo2Transaction_s
{
  AdblSession session;  // reference
  
  EcUdc config;         // reference
};

//-----------------------------------------------------------------------------

Adbo2Transaction adbo2_trx_create (AdblSession session, EcUdc config)
{
  Adbo2Transaction self = ENTC_NEW (struct Adbo2Transaction_s);
  
  self->session = session;
  self->config = config;
  
  // start transaction in the database
  adbl_dbbegin (self->session);
  
  return self;
}

//-----------------------------------------------------------------------------

void adbo2_trx_destroy (Adbo2Transaction* pself)
{
  Adbo2Transaction self = *pself;
  

  
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

const char* adbo2_trx_getColName (EcUdc knode)
{
  const EcString key = NULL;
        
  switch (ecudc_type (knode))
  {
    case ENTC_UDC_NODE:    // default config
    {
      key = ecudc_name (knode);
      break;
    }
    case ENTC_UDC_STRING:  // override 
    {
      key = ecudc_asString (knode);
      break;
    }
    default:
    {
      eclog_msg (LL_ERROR, "ADBO_2", "constraint", "type for key is not supported");
      // error
      return NULL;
    }
  }

  return key;
}

//-----------------------------------------------------------------------------

void adbo2_trx_addConstraint (EcUdc knode, EcUdc param, AdblConstraint* constraints)
{
  const EcString key = adbo2_trx_getColName (knode);
  
  // check key
  if (key == NULL)
  {
    eclog_msg (LL_ERROR, "ADBO_2", "constraint", "can't retrieve the column name");

    return;
  }
  
  // **** map UDC value to constraint ***
  
  switch (ecudc_type (param))
  {
    case ENTC_UDC_NUMBER:
    {
      adbl_constraint_addLong (constraints, key, QUOMADBL_CONSTRAINT_EQUAL, ecudc_asNumber (param));
      break;
    }
    case ENTC_UDC_STRING:
    {
      adbl_constraint_addChar (constraints, key, QUOMADBL_CONSTRAINT_EQUAL, ecudc_asString (param));      
      break;
    }
    case ENTC_UDC_BOOL:
    {
      adbl_constraint_addLong (constraints, key, QUOMADBL_CONSTRAINT_EQUAL, ecudc_asBool (param) ? 1 : 0);      
      break;
    }
    case ENTC_UDC_DOUBLE:
    {
      adbl_constraint_addChar (constraints, key, QUOMADBL_CONSTRAINT_EQUAL, ecudc_asString (param));      
      break;
    }
    case ENTC_UDC_NODE:
    {
      // TODO: 
      break;
    }
    case ENTC_UDC_LIST:
    {
      // TODO:
      break;
    }
    default:
    {
     
      // not supported
    }
  }
}

//-----------------------------------------------------------------------------

int adbo2_trx_check_pk (EcUdc tconfig, EcUdc params, AdblConstraint* constraints)
{
  int pksActive = 0;
  int pksFound = 0;
  
  EcUdc pkeys = ecudc_node (tconfig, "pk");
  if (pkeys)
  {
    void* cursor = NULL;
    EcUdc pk;
    
    for (pk = ecudc_next (pkeys, &cursor); pk; pk = ecudc_next (pkeys, &cursor))
    {
      EcUdc param = ecudc_node (params, ecudc_name (pk));
      if (param)
      {
        adbo2_trx_addConstraint (pk, param, constraints);
        pksFound++;
      }
      
      pksActive++;
    }
  }
  
  return pksFound;
}

//-----------------------------------------------------------------------------

int adbo2_trx_check_fk (EcUdc tconfig, EcUdc params, AdblConstraint* constraints)
{
  int pksActive = 0;
  int pksFound = 0;
  
  EcUdc fkeys = ecudc_node (tconfig, "fk");
  if (fkeys)
  {
    void* cursor = NULL;
    EcUdc pk;
    
    for (pk = ecudc_next (fkeys, &cursor); pk; pk = ecudc_next (fkeys, &cursor))
    {
      EcUdc param = ecudc_node (params, ecudc_name (pk));
      if (param)
      {
        adbo2_trx_addConstraint (pk, param, constraints);
        pksFound++;
      }
      
      pksActive++;
    }
  }
  
  return pksFound;
}

//-----------------------------------------------------------------------------

void adbo2_trx_add_columns (EcUdc tconfig, AdblQuery* query)
{
  EcUdc cols = ecudc_node (tconfig, "cols");
  if (cols)
  {
    void* cursor = NULL;
    EcUdc col;
    
    for (col = ecudc_next (cols, &cursor); col; col = ecudc_next (cols, &cursor))
    {
      const EcString key = adbo2_trx_getColName (col);
  
      // check key
      if (key == NULL)
      {
        eclog_msg (LL_ERROR, "ADBO_2", "constraint", "can't retrieve the column name");

        return;
      }      
      
      adbl_query_addColumn (query, key, 0);
    }
  }
}

//-----------------------------------------------------------------------------

int adbo2_trx_query (Adbo2Transaction self, const EcString table, EcUdc params, EcUdc data, EcErr err)
{
  AdblSecurity adblsec;      

  // find table in config
  EcUdc tconfig = ecudc_node (self->config, table);
  if (tconfig == NULL)
  {
    return ecerr_set_fmt(err, ENTC_LVL_ERROR, ENTC_ERR_NOT_FOUND, "can't find table '%s' in ADBO config", table);
  }
    
  AdblQuery* query = adbl_query_new ();
  AdblConstraint* constraints = adbl_constraint_new (QUOMADBL_CONSTRAINT_AND);
    
  // check for primary key (PK)
  int pksFound = adbo2_trx_check_pk (tconfig, params, constraints);
  if (pksFound == 0)
  {
    // check for foreign keys
    int fksFound = adbo2_trx_check_fk (tconfig, params, constraints);
    if (fksFound == 0)
    {
      return ecerr_set (err, ENTC_LVL_ERROR, ENTC_ERR_MISSING_PARAM, "no primary or foreign keys found in parameters");
    }
  }
  
  adbl_query_setTable (query, table);
  
  adbl_query_setConstraint (query, constraints);
  
  // add columns
  adbo2_trx_add_columns (tconfig, query);
  
  AdblCursor* cursor = adbl_dbquery (self->session, query, &adblsec);
  if (cursor)
  {
    while (adbl_dbcursor_next (cursor))
    {
      EcUdc row = ecudc_create (EC_ALLOC, ENTC_UDC_NODE, NULL);

      EcListCursor lcursor; eclist_cursor_init (query->columns, &lcursor, LIST_DIR_NEXT);
      
      while (eclist_cursor_next(&lcursor))
      {
        AdblQueryColumn* qc = eclist_data (lcursor.node);
        
        const char* colName = qc->column;
        const char* colText = adbl_dbcursor_data (cursor, lcursor.position);
       
        ecudc_add_asString (EC_ALLOC, row, colName, colText);
      }
      
      ecudc_add (data, &row);
    }
    
    adbl_dbcursor_release (&cursor);  
  }
  
  adbl_constraint_delete (&constraints);
  adbl_query_delete (&query);
  
  return ENTC_ERR_NONE;
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

int adbo2_trx_insertOrUpdate  (Adbo2Transaction self, const EcString table, EcUdc params, EcUdc data, EcErr err)
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
    return ENTC_ERR_NONE;
  }
}

//-----------------------------------------------------------------------------

struct Adbo2Session_s
{
  AdblSession session; 
  
  EcUdc config;
};

//-----------------------------------------------------------------------------

Adbo2Session adbo2_session_create (struct AdblManager_s* adblm, const EcString jsonConfig, const EcString entity)
{
  EcUdc config;
  
  // read config from file
  int res = ecjson_readFromFile (jsonConfig, &config, NULL, 0);
  if (res)
  {
    return NULL;
  }
  
  {
    Adbo2Session self = ENTC_NEW (struct Adbo2Session_s);
    
    self->session = adbl_openSession (adblm, entity);
    self->config = config;
    
    return self;
  }
}

//-----------------------------------------------------------------------------

void adbo2_session_destroy (Adbo2Session* pself)
{
  Adbo2Session self = *pself;
  
  if (self->session)
  {
    adbl_closeSession (&(self->session)); 
  }
  
  if (self->config)
  {    
    ecudc_destroy(EC_ALLOC, &(self->config));
  }
  
  ENTC_DEL (pself, struct Adbo2Session_s);
}

//-----------------------------------------------------------------------------

Adbo2Transaction adbo2_session_transaction (Adbo2Session self)
{
  return adbo2_trx_create (self->session, self->config);
}

//-----------------------------------------------------------------------------

struct Adbo2_s
{
  struct AdblManager_s* adblm;
  
  EcString confPath;
};

//-----------------------------------------------------------------------------

Adbo2 adbo2_create (const EcString confPath, const EcString binPath, const EcString adboSubPath)
{
  Adbo2 self = ENTC_NEW (struct Adbo2_s);
  
  // create a new instance
  self->adblm = adbl_new ();
  
  if (adboSubPath)
  {
    self->confPath = ecfs_mergeToPath (confPath, adboSubPath);
  }
  else
  {
    self->confPath = ecstr_copy (confPath);
  }
  
  // create folder
  ecfs_createDirIfNotExists (self->confPath);
  
  // apply config
  adbl_scanJson (self->adblm, confPath, binPath);
  
  return self;
}

//-----------------------------------------------------------------------------

void adbo2_destroy (Adbo2* pself)
{
  Adbo2 self = *pself;
  
  adbl_delete (&(self->adblm));
  
  ecstr_delete (&(self->confPath));
  
  ENTC_DEL (pself, struct Adbo2_s);
}

//-----------------------------------------------------------------------------

Adbo2Session adbo2_session_get (Adbo2 self, const EcString jsonConf, const EcString entity)
{
  EcString jsonConfigFile = ecfs_mergeToPath (self->confPath, jsonConf == NULL ? "adbo.json" : jsonConf);
   
  Adbo2Session ret = adbo2_session_create (self->adblm, jsonConfigFile, entity == NULL ? "default" : entity);
  
  ecstr_delete (&jsonConfigFile);
  
  return ret;
}

//-----------------------------------------------------------------------------
