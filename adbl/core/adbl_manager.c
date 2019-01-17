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

#include "adbl_manager.h"

// entc includes
#include <sys/entc_mutex.h>

#include <system/ecfile.h>
#include <system/ecdl.h>
#include <types/ecalloc.h>

#include <types/ecstream.h>
#include <types/ecmap.h>

#include <tools/ecxmlstream.h>
#include <tools/ecjson.h>

#include "adbl_security.h"

#define MODULE "ADBL"

typedef struct 
{
  EcLibraryHandle handle;
  
  const AdblModuleInfo* dbinfo;

  adbl_dbconnect_t dbconnect;
  
  adbl_dbdisconnect_t dbdisconnect;
  
  adbl_dbquery_t dbquery;
  
  adbl_dbprocedure_t dbprocedure;
  
  adbl_dbtable_size_t dbtable_size;
  
  adbl_dbupdate_t dbupdate;
  
  adbl_dbinsert_t dbinsert;
  
  adbl_dbdelete_t dbdelete;
  
  adbl_dbbegin_t dbbegin;
  
  adbl_dbcommit_t dbcommit;
  
  adbl_dbrollback_t dbrollback;
  
  adbl_dbcursor_next_t dbcursor_next;
  
  adbl_dbcursor_data_t dbcursor_data;
  
  adbl_dbcursor_nextdata_t dbcursor_nextdata;
  
  adbl_dbcursor_release_t dbcursor_release;
  
  adbl_dbsequence_get_t dbsequence_get;
  
  adbl_dbsequence_release_t dbsequence_release;
  
  adbl_dbsequence_next_t dbsequence_next;
  
  adbl_dbschema_t dbschema;
  
  adbl_dbtable_t dbtable;
  
} ADBLModuleProperties;

typedef struct AdblSessionPool_s* AdblSessionPool;

typedef struct
{
  
  EcString type;
  
  AdblConnectionProperties properties;

  ADBLModuleProperties* pp;
  
  AdblSessionPool pool;
  
} AdblCredentials;


//-----------------------------------------------------------------------------

struct AdblSession_s
{
  
  ADBLModuleProperties* pp;   // reference
  
  AdblSessionPool pool;       // reference
  
  void* connection;
  
  int isFree;
  
};

//-----------------------------------------------------------------------------

struct AdblManager_s
{
  
  EntcMutex mutex;
  
  EcMap credentials;
  
  EcMap modules;
  
  EcString path;
  
};

//-----------------------------------------------------------------------------

struct AdblSessionPool_s
{
  
  EcList pool;
  
  int minPoolSize;
  
  AdblCredentials* pc;
  
  EntcMutex mutex;
  
};

//-----------------------------------------------------------------------------

static int __STDCALL adbl_sessionpool_onDestroy (void* ptr)
{
  AdblSession session = ptr;
  
  // disconnect from database 
  if (isAssigned (session->connection) && isAssigned (session->pp))
  {
    if (isAssigned(session->pp->dbdisconnect))
    {
      session->pp->dbdisconnect (session->connection);
    }
    else
    {
      eclog_msg (LL_WARN, MODULE, "delete", "disconnect method in matrix is not defined");
    }
  }
    
  ENTC_DEL (&session, struct AdblSession_s);
  
  return 0;
}

//-----------------------------------------------------------------------------

AdblSessionPool adbl_sessionpool_create (int minPoolSize, AdblCredentials* pc)
{
  AdblSessionPool self = ENTC_NEW (struct AdblSessionPool_s);
  
  self->minPoolSize = minPoolSize;
  self->pool = eclist_create (adbl_sessionpool_onDestroy);
  
  self->pc = pc;
  
  self->mutex = entc_mutex_new();
  
  return self;
}

//-----------------------------------------------------------------------------

void adbl_sessionpool_destroy (AdblSessionPool* pself)
{
  AdblSessionPool self = *pself;
  
  eclist_destroy (&(self->pool));
  
  entc_mutex_del(&(self->mutex));
  
  ENTC_DEL (pself, struct AdblSessionPool_s);
}

//-----------------------------------------------------------------------------

AdblSession adbl_sessionpool_get (AdblSessionPool self)
{
  ADBLModuleProperties* pp = self->pc->pp;
  AdblSession session = NULL;
  
  entc_mutex_lock (self->mutex);

  // iterrate through the pool to find the next free session
  {
    EcListCursor cursor; eclist_cursor_init (self->pool, &cursor, LIST_DIR_NEXT);
    
    while (eclist_cursor_next (&cursor))
    {
      session = eclist_data (cursor.node);
      
      if (session->isFree)
      {
        // assign this one to our session
        session->isFree = FALSE;
        
        break;
      }
      
      session = NULL;
    }
      
    if (session == NULL)
    {
      eclog_fmt (LL_TRACE, MODULE, "connect", "try to connect to database [%s]", self->pc->type);
  
      if (isNotAssigned (pp))
      {
        eclog_msg (LL_ERROR, MODULE, "connect", "credentials without database");
      }
      else
      {
        // create a new session
        session = ENTC_NEW (struct AdblSession_s);
        
        // session is used
        session->isFree = FALSE;
        
        // add reference to backend methods
        session->pp = pp;
        session->pool = self;

        if (isAssigned (pp->dbconnect))
        {
          session->connection = pp->dbconnect (&(self->pc->properties));
        }
        else
        {
          eclog_msg (LL_WARN, MODULE, "connect", "connect method in matrix is not defined");
        }
        
        if (session->connection == NULL)
        {
          eclog_fmt (LL_ERROR, MODULE, "connect", "can't connect to database [%s]", self->pc->type);
          
          adbl_sessionpool_onDestroy (session);
          
          session = NULL;
        }
        else
        {
          eclist_push_back (self->pool, session);
        }
      }
    }
  }
  
  entc_mutex_unlock (self->mutex);
  
  return session;
}

//-----------------------------------------------------------------------------

void adbl_sessionpool_release (AdblSession* psession)
{
  AdblSession session = *psession;
  AdblSessionPool self = session->pool;
  
  entc_mutex_lock (self->mutex);

  session->isFree = TRUE;
  
  // TODO: cleanup if sessions > min size

  entc_mutex_unlock (self->mutex);
}

//=============================================================================

struct AdblCursor_p
{

  void* ptr;
  
  ADBLModuleProperties* pp;
  
};

struct AdblSequence_p
{
  
  void* ptr;

  ADBLModuleProperties* pp;  /* reference */

};



/* method definition */

/*------------------------------------------------------------------------*/

AdblCredentials* adbl_credentials_new (const EcString dbtype)
{
  AdblCredentials* self = ENTC_NEW(AdblCredentials);
  
  self->type = ecstr_copy(dbtype);

  //inisitalise the properties
  self->properties.port = 0;
  self->properties.host = 0;        
  self->properties.file = 0;
  self->properties.schema = 0;
  self->properties.username = 0;
  self->properties.password = 0;
  
  self->pp = NULL;
  
  self->pool = adbl_sessionpool_create (5, self);
  
  return self;
}

/*------------------------------------------------------------------------*/

static void __STDCALL adbl_modules_onDestroy (void* key, void* val)
{
  {
    EcString h = key; ecstr_delete (&h);
  }
  {
    ADBLModuleProperties* properties = val;
    
    ecdl_delete(&(properties->handle));
    
    ENTC_DEL (&properties, ADBLModuleProperties);
  }
}

/*------------------------------------------------------------------------*/

static void __STDCALL adbl_credentials_onDestroy (void* key, void* val)
{
  {
    EcString h = key; ecstr_delete (&h);
  }
  {
    AdblCredentials* pc = val;
    
    adbl_sessionpool_destroy(&(pc->pool));

    /* clean */
    pc->properties.port = 0;
    ecstr_delete( &(pc->properties.host) );
    ecstr_delete( &(pc->properties.file) );
    ecstr_delete( &(pc->properties.schema) );
    ecstr_delete( &(pc->properties.username) );
    ecstr_delete( &(pc->properties.password) );
    
    ecstr_delete( &(pc->type) );    
    
    ENTC_DEL (&pc, AdblCredentials);
  }
}

/*------------------------------------------------------------------------*/

AdblManager adbl_new ()
{
  AdblManager self = ENTC_NEW(struct AdblManager_s);
    
  self->mutex = entc_mutex_new ();
  self->modules = ecmap_create (NULL, adbl_modules_onDestroy);
  self->credentials = ecmap_create (NULL, adbl_credentials_onDestroy);
  self->path = ecstr_init();
  
  return self;
}

/*------------------------------------------------------------------------*/

void adbl_delete (AdblManager* ptr)
{
  AdblManager self = *ptr;
  
  entc_mutex_del (&(self->mutex));

  ecmap_destroy (&(self->credentials));
  ecmap_destroy (&(self->modules));
  
  ecstr_delete(&(self->path));

  ENTC_DEL( ptr, struct AdblManager_s );
}

/*------------------------------------------------------------------------*/

void adbl_setCredentialsFile (AdblManager self, const EcString name, const EcString dbtype, const EcString file)
{
  /* variables */
  EcMapNode node;
  ADBLModuleProperties* pp;
  AdblCredentials* pc;  

  entc_mutex_lock(self->mutex);
  
  node = ecmap_find (self->modules, (void*)dbtype);
  if (node == NULL)
  {
    eclog_fmt (LL_WARN, MODULE, "credentials", "database '%s' not in the list", dbtype);

    entc_mutex_unlock(self->mutex);
    return;
  }
  
  pp = ecmap_node_value (node);
  
  node = ecmap_find (self->credentials, (void*)name);
  if (node == NULL)
  { 
    pc = adbl_credentials_new (dbtype);  
    //add to map
    ecmap_insert (self->credentials, ecstr_copy(name), pc);
  }
  else
  {
    pc = ecmap_node_value (node);
    ecstr_replace(&(pc->type), dbtype);
  }
  
  pc->pp = pp;
  
  ecstr_replace(&(pc->properties.file), file);

  entc_mutex_unlock(self->mutex);
}

/*------------------------------------------------------------------------*/

void adbl_addPlugin (AdblManager self, EcLibraryHandle handle, const char* name)
{
  EcMapNode node;
  ADBLModuleProperties* properties;

  node = ecmap_find (self->modules, (void*)name);
  if (node)
  {
    ecdl_delete(&handle);
    // already exists
    return;
  }
  
  //create new credential
  properties = ENTC_NEW (ADBLModuleProperties);  
  
  properties->handle = handle;
  
  // check the methods
  properties->dbconnect          = (adbl_dbconnect_t)          ecdl_method (handle, "dbconnect");
  properties->dbdisconnect       = (adbl_dbdisconnect_t)       ecdl_method (handle, "dbdisconnect");
  properties->dbquery            = (adbl_dbquery_t)            ecdl_method (handle, "dbquery");
  properties->dbprocedure        = (adbl_dbprocedure_t)        ecdl_method (handle, "dbprocedure");
  properties->dbtable_size       = (adbl_dbtable_size_t)       ecdl_method (handle, "dbtable_size");
  properties->dbupdate           = (adbl_dbupdate_t)           ecdl_method (handle, "dbupdate");
  properties->dbinsert           = (adbl_dbinsert_t)           ecdl_method (handle, "dbinsert");
  properties->dbdelete           = (adbl_dbdelete_t)           ecdl_method (handle, "dbdelete");
  properties->dbbegin            = (adbl_dbbegin_t)            ecdl_method (handle, "dbbegin");
  properties->dbcommit           = (adbl_dbcommit_t)           ecdl_method (handle, "dbcommit");
  properties->dbrollback         = (adbl_dbrollback_t)         ecdl_method (handle, "dbrollback");
  properties->dbcursor_next      = (adbl_dbcursor_next_t)      ecdl_method (handle, "dbcursor_next");
  properties->dbcursor_data      = (adbl_dbcursor_data_t)      ecdl_method (handle, "dbcursor_data");
  properties->dbcursor_nextdata  = (adbl_dbcursor_nextdata_t)  ecdl_method (handle, "dbcursor_nextdata");
  properties->dbcursor_release   = (adbl_dbcursor_release_t)   ecdl_method (handle, "dbcursor_release");
  properties->dbsequence_get     = (adbl_dbsequence_get_t)     ecdl_method (handle, "dbsequence_get");
  properties->dbsequence_release = (adbl_dbsequence_release_t) ecdl_method (handle, "dbsequence_release");
  properties->dbsequence_next    = (adbl_dbsequence_next_t)    ecdl_method (handle, "dbsequence_next");
  properties->dbschema           = (adbl_dbschema_t)           ecdl_method (handle, "dbschema");
  properties->dbtable            = (adbl_dbtable_t)            ecdl_method (handle, "dbtable");
  
  //add to map
  ecmap_insert (self->modules, ecstr_copy(name), properties);
  
  eclog_fmt (LL_DEBUG, MODULE, "init", "adbl plugin %s successful loaded", name );
}

/*------------------------------------------------------------------------*/

void adbl_scanPlugin (AdblManager self, const EcString filename)
{
  EcLibraryHandle handle;
  adbl_info_t info;
    
  handle = ecdl_new (filename);
  if( !handle )
  {
    // cannot be loaded for some reason
    return;
  }
  
  // check first the name and info method
  info = (adbl_info_t)ecdl_method (handle, "dbinfo");
  if (isNotAssigned (info))
  {
    ecdl_delete(&handle);
    // not an adbl plugin
    return;
  }
  
  {
    // fetch the module info
    const AdblModuleInfo* moduleinfo = info();
    
    entc_mutex_lock (self->mutex);
    
    adbl_addPlugin (self, handle, moduleinfo->name);
    
    entc_mutex_unlock (self->mutex);
  }
}

//-----------------------------------------------------------------------------

static int __STDCALL adbl_validate_config_engines_onDestroy (void* ptr)
{
  EcString filename = ptr;

  ecstr_delete (&filename);
  
  return 0;
}

//----------------------------------------------------------------------------------------

void adbl_validate_config (AdblManager self, const EcString configpath, const EcString execpath)
{
  // check some things
  if (ecstr_empty(self->path))
  {
    if (ecstr_valid (execpath))
    {
      EcString path1 = ecfs_mergeToPath (execpath, "../adbl");
      
      EcString path2 = ecfs_getRealPath (path1);
      
      ecstr_replaceTO (&(self->path), path2);
      
      ecstr_delete (&path1);
    }
  }
  
  if (ecstr_valid(self->path))
  {
    EcList engines = eclist_create (adbl_validate_config_engines_onDestroy);
    
    //eclog_fmt (LL_TRACE, MODULE, "scan", "scan path '%s' for adbl modules", self->path);
    
    // fill a list with all files in that directory
    if (!ecdh_scan(self->path, engines, ENTC_FILETYPE_ISFILE))
    {
      eclog_fmt (LL_ERROR, MODULE, "scan", "can't find path '%s'", ecstr_cstring(self->path) );
    }
    
    {
      EcListCursor cursor;
      eclist_cursor_init (engines, &cursor, LIST_DIR_NEXT);
      
      while (eclist_cursor_next (&cursor))
      {
        EcString filename = eclist_data (cursor.node);
        // scan the library
        adbl_scanPlugin (self, filename);
      }
    }
    
    // clean up
    eclist_destroy (&engines);
  }
  else
  {
    eclog_msg (LL_ERROR, MODULE, "scan", "no scanpath defined in config");
  }
}

//----------------------------------------------------------------------------------------

void adbl_scanJsonFromFile (const EcString configpath, EcUdc* pdata)
{
  int res;
  EcString filename = ecfs_mergeToPath (configpath, "adbl.json");
  
  //eclog_fmt (LL_TRACE, MODULE, "scan", "using config '%s'", filename);
  
  res = ecjson_readFromFile (filename, pdata, NULL, 0);
  if (res)
  {
    eclog_fmt (LL_WARN, MODULE, "scan", "can't read json file '%s'", filename);

    ecstr_delete(&filename);

    return;
  }
  
  ecstr_delete(&filename);
}

//----------------------------------------------------------------------------------------

void adbl_scanJsonParse (AdblManager self, EcUdc data, const EcString configpath, const EcString execpath)
{
  EcUdc databases;
  void* cursor = NULL;
  EcUdc item;

  ecstr_replace (&(self->path), ecudc_get_asString(data, "path", NULL));
  
  adbl_validate_config (self, configpath, execpath);
  
  databases = ecudc_node(data, "databases");
  if (databases == NULL)
  {
    eclog_fmt (LL_WARN, MODULE, "scan", "can't find databases in json");
    return;
  }
  
  for (item = ecudc_next(databases, &cursor); item; item = ecudc_next(databases, &cursor))
  {
    const EcString dbname = ecudc_get_asString(item, "name", NULL);
    const EcString dbtype = ecudc_get_asString(item, "type", NULL);
    
    eclog_fmt (LL_TRACE, MODULE, "scan", "found name = '%s', type = '%s'", dbname, dbtype);
    
    if (dbname && dbtype)
    {
      ADBLModuleProperties* pp;
      EcMapNode node;
      
      node = ecmap_find (self->modules, (void*)dbtype);
      if (node == NULL)
      {
        eclog_fmt (LL_WARN, MODULE, "credentials", "database '%s' not in the list", dbtype);
        return;
      }
      
      pp = ecmap_node_value (node);
      
      node = ecmap_find (self->credentials, (void*)dbname);
      if (node)
      {
        eclog_fmt (LL_WARN, MODULE, "credentials", "parsing the config file: db-source already exists [%s] in current register", dbname );
        continue;
      }
      
      {
        EcUdc file;

        //create new credential
        AdblCredentials* pc = adbl_credentials_new (dbtype);
        pc->pp = pp;
        //add to map
        ecmap_insert (self->credentials, ecstr_copy(dbname), pc);
        //parse the other stuff
        
        eclog_fmt (LL_TRACE, MODULE, "scan", "added '%s'", dbname);
        
        pc->properties.host = ecstr_copy (ecudc_get_asString(item, "host", NULL));
        pc->properties.port = ecudc_get_asNumber (item, "port", 0);
        
        pc->properties.username = ecstr_copy (ecudc_get_asString(item, "user", NULL));
        pc->properties.password = ecstr_copy (ecudc_get_asString(item, "pass", NULL));
        
        pc->properties.schema = ecstr_copy (ecudc_get_asString(item, "schema", NULL));
        
        file = ecudc_node (item, "file");
        if (file)
        {
          const EcString fileprefix = ecudc_get_asString(item, "prefix", NULL);
          const EcString filextension = ecudc_get_asString(item, "ext", NULL);
          
          EcStream file = ecstream_create ();
          
          if( fileprefix )
          {
            ecstream_append_str ( file, fileprefix );
            ecstream_append_str ( file, "_" );
          }
          
          if( pc->properties.schema )
          {
            ecstream_append_str ( file, pc->properties.schema );
          }
          
          if( filextension )
          {
            ecstream_append_str ( file, filextension );
          }
          else
          {
            ecstream_append_str ( file, ".db" );
          }
          
          pc->properties.file = ecfs_mergeToPath (configpath, ecstream_get( file ));
          
          /* clean up */
          ecstream_destroy (&file);
        }
      }
    }
  }
}

//----------------------------------------------------------------------------------------

void adbl_scanJson (AdblManager self, const EcString configpath, const EcString execpath)
{
  EcUdc data = NULL;
  
  adbl_scanJsonFromFile (configpath, &data);

  if (data)
  {
    adbl_scanJsonParse (self, data, configpath, execpath);
    
    ecudc_destroy(EC_ALLOC, &data);
  }
}

/*------------------------------------------------------------------------*/

AdblSession adbl_openSession (AdblManager self, const char* dbsource)
{
  /* variable definition */
  EcMapNode node;
  AdblCredentials* pc;

  if (isNotAssigned(dbsource))
  {
    eclog_msg (LL_WARN, MODULE, "session", "db-source was NULL" );
    return 0;  
  }
  
  node = ecmap_find (self->credentials, (void*)dbsource);
  if (node == NULL)
  {
    /* not found */
    eclog_fmt (LL_WARN, MODULE, "session", "can't find db-source [%s] in current register", dbsource);
    return 0;
  }

  entc_mutex_lock(self->mutex);
  
  pc = ecmap_node_value (node);  

  entc_mutex_unlock(self->mutex);

  return adbl_sessionpool_get (pc->pool);
}

/*------------------------------------------------------------------------*/

void adbl_closeSession (AdblSession* pself)
{
  adbl_sessionpool_release (pself);
}

/*------------------------------------------------------------------------*/

AdblCursor* adbl_dbquery (AdblSession session, AdblQuery* query, AdblSecurity* security)
{
  ADBLModuleProperties* pp;
  
  if (session == NULL)
  {
    eclog_msg (LL_ERROR, MODULE, "begin", "no session");
    return NULL;
  }
  
  pp = session->pp;

  void* cc;
  AdblCursor* cursor;

  if (isNotAssigned (pp))
  {
    eclog_msg (LL_ERROR, MODULE, "dbquery", "credentials without database" );
    return NULL;
  }
  
  if (isNotAssigned(session->connection))
  {
    eclog_msg (LL_WARN, MODULE, "dbquery", "no active database connection" );
    return NULL;
  }
  
  if (isNotAssigned (pp->dbquery))
  {
    eclog_msg (LL_WARN, MODULE, "dbquery", "query method in matrix is not defined" );
    return NULL;
  }
  
  /* check for faulty sql */
  adbl_query_sec (query, security);
      
  if( security->inicident )
  {
    return NULL;  
  }

  cc = pp->dbquery (session->connection, query);
  if(!cc)
  {
    return NULL;  
  }

  cursor = ENTC_NEW(struct AdblCursor_p);
  
  cursor->ptr = cc;
  cursor->pp = pp;
  
  return cursor;
}

/*------------------------------------------------------------------------*/

int adbl_dbprocedure (AdblSession session, AdblProcedure* procedure, AdblSecurity* security)
{
  ADBLModuleProperties* pp;
  
  if (session == NULL)
  {
    eclog_msg (LL_ERROR, MODULE, "begin", "no session");
    return 0;
  }
  
  pp = session->pp;

  if (isNotAssigned (pp))
  {
    eclog_msg (LL_ERROR, MODULE, "dbquery", "credentials without database" );
    return 0;
  }

  if (isNotAssigned(session->connection))
  {
    eclog_msg (LL_WARN, MODULE, "dbquery", "no active database connection" );
    return 0;
  }

  if (isNotAssigned (pp->dbprocedure))
  {
    eclog_msg (LL_WARN, MODULE, "dbquery", "procedure method in matrix is not defined" );
    return 0;
  }

  // TODO
  
  /*
  if( security->inicident )
  {
    return 0;
  }
   */

  return pp->dbprocedure (session->connection, procedure);
}

/*------------------------------------------------------------------------*/

uint_t adbl_table_size (AdblSession session, const EcString table)
{
  ADBLModuleProperties* pp;
  
  if (session == NULL)
  {
    eclog_msg (LL_ERROR, MODULE, "begin", "no session");
    return 0;
  }
  
  pp = session->pp;

  if (isNotAssigned (pp))
  {
    eclog_msg (LL_ERROR, MODULE, "size", "credentials without database" );
    return 0;
  }
  
  if (isAssigned(session->connection))
  {
    if (isAssigned(pp->dbtable_size))
    {
      return pp->dbtable_size (session->connection, table);
    }
  }
  return 0;
}

/*------------------------------------------------------------------------*/

AdblSequence* adbl_dbsequence_get( AdblSession session, const EcString table )
{
  ADBLModuleProperties* pp;
  
  if (session == NULL)
  {
    eclog_msg (LL_ERROR, MODULE, "begin", "no session");
    return NULL;
  }
  
  pp = session->pp;

  if (isNotAssigned (pp))
  {
    eclog_msg (LL_ERROR, MODULE, "sequence", "credentials without database" );
    return NULL;
  }
  
  if (isAssigned(session->connection))
  {
    if (isAssigned(pp->dbsequence_get))
    {
      void* ptr = pp->dbsequence_get (session->connection, table);
      
      if( ptr )
      {
        AdblSequence* sequence = ENTC_NEW(AdblSequence);
        
        sequence->pp = pp;        
        sequence->ptr = ptr;
        
        return sequence;        
      }
    }
  }
  return 0;  
}

/*------------------------------------------------------------------------*/

void adbl_sequence_release (AdblSequence** ptr)
{
  AdblSequence* self = *ptr;
  /* clean up the adbl plugin */
  if (isAssigned(self->pp->dbsequence_release))
  {
    self->pp->dbsequence_release(self->ptr);
  }
  /* release local */
  ENTC_DEL( ptr, AdblSequence );
}

/*------------------------------------------------------------------------*/

uint_t adbl_sequence_next (AdblSequence* self)
{
  if (isNotAssigned (self))
  {
    return 0;
  }
  
  if (isNotAssigned (self->pp))
  {
    return 0;
  }
  
  if (isNotAssigned (self->pp->dbsequence_next))
  {
    return 0;  
  }

  return self->pp->dbsequence_next (self->ptr);
}

/*------------------------------------------------------------------------*/

int adbl_dbcursor_next (AdblCursor* cursor)
{
  if (isNotAssigned(cursor))
  {
    // query was not successfull
    return FALSE;
  }
  
  if (isAssigned(cursor->pp->dbcursor_next))
  {
    return cursor->pp->dbcursor_next (cursor->ptr);
  }
    
  return FALSE;
}

/*------------------------------------------------------------------------*/

const char* adbl_dbcursor_data (AdblCursor* cursor, uint_t column)
{
  if (isAssigned(cursor->pp->dbcursor_data))
  {
    return cursor->pp->dbcursor_data (cursor->ptr, column);
  }
  
  return "";  
}

/*------------------------------------------------------------------------*/

const char* adbl_dbcursor_nextdata (AdblCursor* cursor)
{
  if (isAssigned (cursor->pp->dbcursor_nextdata))
  {
    return cursor->pp->dbcursor_nextdata (cursor->ptr);
  }
  
  return "";
}

/*------------------------------------------------------------------------*/

void adbl_dbcursor_release (AdblCursor** ptr)
{
  AdblCursor* cursor = *ptr;
  
  if (isAssigned(cursor->pp->dbcursor_release))
  {
    cursor->pp->dbcursor_release (cursor->ptr);
  }
  
  ENTC_DEL( ptr, AdblCursor );
}

/*------------------------------------------------------------------------*/

int adbl_dbupdate (AdblSession session, AdblUpdate* update, int isInsert, AdblSecurity* security)
{
  ADBLModuleProperties* pp;
  
  if (session == NULL)
  {
    eclog_msg (LL_ERROR, MODULE, "begin", "no session");
    return 0;
  }
  
  pp = session->pp;

  if (isNotAssigned (pp))
  {
    eclog_msg (LL_ERROR, MODULE, "update", "credentials without database" );
    return 0;
  }
  
  if (isNotAssigned(session->connection))
  {
    eclog_msg (LL_WARN, MODULE, "update", "no active database connection" );
    return 0;
  }
  
  if (isNotAssigned(pp->dbupdate))
  {
    eclog_msg (LL_WARN, MODULE, "update", "update method in matrix is not defined" );
    return 0;
  }
  
  /* check for faulty sql */
  adbl_update_sec (update, security);
  
  if( security->inicident )
  {
    return 0;  
  }
  
  return pp->dbupdate (session->connection, update, isInsert);
}

/*------------------------------------------------------------------------*/

int adbl_dbinsert (AdblSession session, AdblInsert* insert, AdblSecurity* security)
{
  ADBLModuleProperties* pp;
  
  if (session == NULL)
  {
    eclog_msg (LL_ERROR, MODULE, "begin", "no session");
    return 0;
  }
  
  pp = session->pp;

  if (isNotAssigned (pp))
  {
    eclog_msg (LL_ERROR, MODULE, "insert", "credentials without database" );
    return 0;
  }
  
  if (isNotAssigned(session->connection))
  {
    eclog_msg (LL_WARN, MODULE, "insert", "no active database connection" );
    return 0;
  }
  
  if (isNotAssigned(pp->dbinsert))
  {
    eclog_msg (LL_WARN, MODULE, "insert", "insert method in matrix is not defined" );
    return 0;
  }
  
  if (security)
  {
    /* check for faulty sql */
    adbl_insert_sec (insert, security);
    
    if( security->inicident )
    {
      return 0;  
    }    
  }
    
  return pp->dbinsert (session->connection, insert);
}

/*------------------------------------------------------------------------*/

int adbl_dbdelete( AdblSession session, AdblDelete* del, AdblSecurity* security )
{
  ADBLModuleProperties* pp;
  
  if (session == NULL)
  {
    eclog_msg (LL_ERROR, MODULE, "begin", "no session");
    return 0;
  }
  
  pp = session->pp;

  if (isNotAssigned (pp))
  {
    eclog_msg (LL_ERROR, MODULE, "delete", "credentials without database" );
    return 0;
  }
  
  if (isNotAssigned(session->connection))
  {
    eclog_msg (LL_WARN, MODULE, "delete", "no active database connection" );
    return 0;
  }
  
  if (isNotAssigned (pp->dbdelete))
  {
    eclog_msg (LL_WARN, MODULE, "delete", "delete method in matrix is not defined" );
    return 0;
  }
  
  if( security )
  {
    /* check for faulty sql */
    adbl_delete_sec (del, security);
    
    if( security->inicident )
    {
      return 0;  
    }    
  }
  
  return pp->dbdelete (session->connection, del);
}

/*------------------------------------------------------------------------*/

void adbl_dbbegin (AdblSession session)
{
  ADBLModuleProperties* pp;
  
  if (session == NULL)
  {
    eclog_msg (LL_ERROR, MODULE, "begin", "no session");
    return;
  }
  
  pp = session->pp;

  if (isNotAssigned (pp))
  {
    eclog_msg (LL_ERROR, MODULE, "begin", "credentials without database" );
    return;
  }
  
  if (isAssigned (session->connection))
  {
    if (isAssigned (pp->dbbegin))
    {
      pp->dbbegin(session->connection);
    }
    else
    {
      eclog_msg (LL_WARN, MODULE, "begin", "begin method in matrix is not defined" );
    }
  }
  else
  {
    eclog_msg (LL_WARN, MODULE, "begin", "no active database connection" );
  }
}

/*------------------------------------------------------------------------*/

void adbl_dbcommit (AdblSession session)
{
  ADBLModuleProperties* pp;
  
  if (session == NULL)
  {
    eclog_msg (LL_ERROR, MODULE, "begin", "no session");
    return;
  }
  
  pp = session->pp;

  if (isNotAssigned (pp))
  {
    eclog_msg (LL_ERROR, MODULE, "commit", "credentials without database" );
    return;
  }
  
  if (isAssigned (session->connection))
  {
    if (isAssigned (pp->dbcommit))
    {
      pp->dbcommit (session->connection);
    }
    else
    {
      eclog_msg (LL_WARN, MODULE, "commit", "commit method in matrix is not defined" );
    }
  }
  else
  {
    eclog_msg (LL_WARN, MODULE, "commit", "no active database connection" );
  }
}

/*------------------------------------------------------------------------*/

void adbl_dbrollback (AdblSession session)
{
  ADBLModuleProperties* pp;
  
  if (session == NULL)
  {
    eclog_msg (LL_ERROR, MODULE, "begin", "no session");
    return;
  }
  
  pp = session->pp;
  
  if (isNotAssigned (pp))
  {
    eclog_msg (LL_ERROR, MODULE, "rollback", "credentials without database" );
    return;
  }
  
  if (isAssigned (session->connection))
  {
    if (isAssigned (pp->dbrollback))
    {
      pp->dbrollback (session->connection);
    }
    else
    {
      eclog_msg (LL_WARN, MODULE, "commit", "rollback method in matrix is not defined" );
    }
  }
  else
  {
    eclog_msg (LL_WARN, MODULE, "commit", "no active database connection" );
  }
}

/*------------------------------------------------------------------------*/

EcList adbl_dbschema (AdblSession session)
{
  ADBLModuleProperties* pp;
  
  if (session == NULL)
  {
    eclog_msg (LL_ERROR, MODULE, "begin", "no session");
    return NULL;
  }
  
  pp = session->pp;

  if (isNotAssigned (pp))
  {
    eclog_msg (LL_ERROR, MODULE, "schema", "credentials without database" );
    return NULL;
  }
  
  if (isAssigned (session->connection))
  {
    if (isAssigned (pp->dbschema))
    {
      return pp->dbschema (session->connection);
    }
    else
    {
      eclog_msg (LL_WARN, MODULE, "schema", "schema method in matrix is not defined" );
    }
  }
  else
  {
    eclog_msg (LL_WARN, MODULE, "schema", "no active database connection" );
  }
  return NULL;
}

/*------------------------------------------------------------------------*/

AdblTable* adbl_dbtable (AdblSession session, const EcString tablename)
{
  ADBLModuleProperties* pp;
  
  if (session == NULL)
  {
    eclog_msg (LL_ERROR, MODULE, "begin", "no session");
    return NULL;
  }
  
  pp = session->pp;

  if (isNotAssigned (pp))
  {
    eclog_msg (LL_ERROR, MODULE, "table", "credentials without database" );
    return NULL;
  }
  
  if (isAssigned (session->connection))
  {
    if (isAssigned (pp->dbtable))
    {
      return pp->dbtable (session->connection, tablename);
    }
    else
    {
      eclog_msg (LL_WARN, MODULE, "table", "table method in matrix is not defined" );
    }
  }
  else
  {
    eclog_msg (LL_WARN, MODULE, "table", "no active database connection" );
  }
  return NULL;  
}

/*------------------------------------------------------------------------*/

