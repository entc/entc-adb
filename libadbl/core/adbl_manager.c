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

#include <system/ecmutex.h>
#include <system/ecfile.h>
#include <system/ecdl.h>
#include <types/ecalloc.h>

#include <types/ecstream.h>
#include <utils/ecobserver.h>

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

typedef struct
{
  
  EcString type;
  
  AdblConnectionProperties properties;

  void* connection;
  
  ADBLModuleProperties* pp;
  
} AdblCredentials;


struct AdblSession_s
{
  
  AdblCredentials* credentials; /* reference */
  
};

struct AdblManager_s
{
  
  EcMutex mutex;
  
  EcFileObserver observer;
  
  EcMap credentials;
  
  EcMap modules;
  
  EcString path;
  
};



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
  //initialise the credentials
  self->connection = 0;
  //inisitalise the properties
  self->properties.port = 0;
  self->properties.host = 0;        
  self->properties.file = 0;
  self->properties.schema = 0;
  self->properties.username = 0;
  self->properties.password = 0;
  
  self->pp = NULL;

  return self;
}

/*------------------------------------------------------------------------*/

void adbl_parseXMLDatabase( AdblManager self, EcXMLStream xmlstream, const char* confpath)
{
  const EcString name = ecxmlstream_nodeAttribute( xmlstream, "name" );
  if(name)
  {
    const EcString dbtype = ecxmlstream_nodeAttribute( xmlstream, "type" );
    if(dbtype)
    {
      ADBLModuleProperties* pp;
      EcMapNode node;
      
      node = ecmap_find(self->modules, dbtype);
      if (node == ecmap_end(self->modules))
      {
        eclogger_fmt (LL_WARN, MODULE, "credentials", "database '%s' not in the list", dbtype);
        return;
      }
      
      pp = ecmap_data (node);
      
      node = ecmap_find(self->credentials, name);      
      if( node == ecmap_end(self->credentials) )
      { 
        int isfile = FALSE;
        EcString fileprefix = NULL;
        EcString filextension = NULL;
        //create new credential
        AdblCredentials* pc = adbl_credentials_new (dbtype);
        pc->pp = pp;
        //add to map
        ecmap_append(self->credentials, name, pc);        
        //parse the other stuff
        ENTC_XMLSTREAM_BEGIN

        if( ecxmlstream_isBegin( xmlstream, "connection" ) )
        {
		        const char* port = ecxmlstream_nodeAttribute( xmlstream, "port" );

          pc->properties.host = ecstr_copy( ecxmlstream_nodeAttribute( xmlstream, "host" ) );
  
          if(port)
            pc->properties.port = atoi(port);

        }
        else if( ecxmlstream_isBegin( xmlstream, "schema" ) )
        {
          pc->properties.schema = ecstr_copy( ecxmlstream_nodeAttribute( xmlstream, "name" ) );
          pc->properties.username = ecstr_copy( ecxmlstream_nodeAttribute( xmlstream, "user" ) );
          pc->properties.password = ecstr_copy( ecxmlstream_nodeAttribute( xmlstream, "password" ) );
        }
        else if( ecxmlstream_isBegin( xmlstream, "file" ) )
        {
          isfile = TRUE;
          fileprefix = ecstr_copy( ecxmlstream_nodeAttribute( xmlstream, "prefix" ) );
          filextension = ecstr_copy( ecxmlstream_nodeAttribute( xmlstream, "filextension" ) );
        }
        ENTC_XMLSTREAM_END( "database" )
        //create the filename
        if( isfile == TRUE )
        {
          EcStream file = ecstream_new();
          
          if( fileprefix )
          {
            ecstream_append( file, fileprefix );
            ecstream_append( file, "_" );
            ecstr_delete (&fileprefix );  
          }

          if( pc->properties.schema )
          {
            ecstream_append( file, pc->properties.schema );
          }

          if( filextension )
          {
            ecstream_append( file, filextension );
            ecstr_delete (&filextension);  
          }
          else
          {
            ecstream_append( file, ".db" );
          }
          
          pc->properties.file = ecfs_mergeToPath(confpath, ecstream_buffer( file ));
          
          /* clean up */
          ecstream_delete(&file);
        }
      }
      else
      {
        eclogger_fmt (LL_WARN, MODULE, "credentials", "parsing the config file: db-source already exists [%s] in current register", name );
      }     
    }
  }  
}

/*------------------------------------------------------------------------*/

void adbl_parseXML( AdblManager self, EcXMLStream xmlstream, const char* confpath, int preread)
{
  ENTC_XMLSTREAM_BEGIN
  
  if (preread && ecxmlstream_isBegin (xmlstream, "modules"))
  {
    ecstr_replace (&(self->path), ecxmlstream_nodeAttribute( xmlstream, "scanpath"));
  }
  else if (!preread && ecxmlstream_isBegin (xmlstream, "database"))
  {
    adbl_parseXMLDatabase(self, xmlstream, confpath);
  }    

  ENTC_XMLSTREAM_END( "adbl" )
}

/*------------------------------------------------------------------------*/

void adbl_parseConfig (AdblManager self, const char* confpath, int preread)
{
  /* open the file */
  EcXMLStream xmlstream = ecxmlstream_openobserver (self->observer);
  /* parse the xml structure */
  while( ecxmlstream_nextNode( xmlstream ) )
  {
    if( ecxmlstream_isBegin( xmlstream, "adbl" ) )
    {
      adbl_parseXML(self, xmlstream, confpath, preread);
    }
  }
  /* close the file */
  ecxmlstream_close (xmlstream);
}

/*------------------------------------------------------------------------*/

AdblManager adbl_new ()
{
  AdblManager self = ENTC_NEW(struct AdblManager_s);
    
  self->mutex = ecmutex_new ();
  self->observer = 0;
  self->modules = ecmap_create (EC_ALLOC);
  self->credentials = ecmap_create (EC_ALLOC);
  self->path = ecstr_init();
  
  return self;
}

/*------------------------------------------------------------------------*/

void adbl_delete (AdblManager* ptr)
{
  AdblManager self = *ptr;
  /* variable declaration */
  EcMapNode node;
  
  ecmutex_delete(&(self->mutex));

  /* delete the observer and close the config file */
  if( self->observer )
  {
    ecf_observer_delete( self->observer );
    self->observer = 0;    
  }
  
  for(node = ecmap_first(self->credentials); node != ecmap_end(self->credentials); node = ecmap_next(node))
  {
    AdblCredentials* pc = ecmap_data(node);
    /* disconnect from database */
    if (isAssigned (pc->connection) && isAssigned(pc->pp))
    {
      if (isAssigned(pc->pp->dbdisconnect))
      {
        pc->pp->dbdisconnect (pc->connection);
      }
      else
      {
        eclogger_msg (LL_WARN, MODULE, "delete", "disconnect method in matrix is not defined"); 
      }
    }
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
  ecmap_destroy (EC_ALLOC, &(self->credentials));

  for(node = ecmap_first(self->modules); node != ecmap_end(self->modules); node = ecmap_next(node))
  {
    ADBLModuleProperties* properties = ecmap_data(node);
    
    ecdl_delete(&(properties->handle));
    
    ENTC_DEL (&properties, ADBLModuleProperties);
  }
  ecmap_destroy (EC_ALLOC, &(self->modules));
  
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

  ecmutex_lock(self->mutex);
  
  node = ecmap_find(self->modules, dbtype);
  if (node == ecmap_end(self->modules))
  {
    eclogger_fmt (LL_WARN, MODULE, "credentials", "database '%s' not in the list", dbtype);

    ecmutex_unlock(self->mutex);
    return;
  }
  
  pp = ecmap_data (node);
  
  node = ecmap_find(self->credentials, name);      
  if( node == ecmap_end(self->credentials) )
  { 
    pc = adbl_credentials_new (dbtype);  
    //add to map
    ecmap_append(self->credentials, name, pc);    
  }
  else
  {
    pc = ecmap_data(node);
    ecstr_replace(&(pc->type), dbtype);
  }
  
  pc->pp = pp;
  
  ecstr_replace(&(pc->properties.file), file);

  ecmutex_unlock(self->mutex);
}

/*------------------------------------------------------------------------*/

void adbl_connect (AdblManager self, AdblCredentials* pc)
{
  eclogger_fmt (LL_DEBUG, MODULE, "connect", "try to connect to database [%s]", pc->type);
  
  if (isNotAssigned (pc->pp))
  {
    eclogger_msg (LL_ERROR, MODULE, "connect", "credentials without database");
    return;
  }
  
  if (isAssigned (pc->pp->dbconnect))
  {
    pc->connection = pc->pp->dbconnect (&(pc->properties));
  }
  else
  {
    eclogger_msg (LL_WARN, MODULE, "connect", "connect method in matrix is not defined");
  }
}

/*------------------------------------------------------------------------*/

void adbl_addPlugin (AdblManager self, EcLibraryHandle handle, const char* name)
{
  EcMapNode node;
  ADBLModuleProperties* properties;

  node = ecmap_find (self->modules, name);      
  if (node != ecmap_end(self->modules))
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
  ecmap_append (self->modules, name, properties);  
  
  eclogger_fmt (LL_DEBUG, MODULE, "init", "adbl plugin %s successful loaded", name );    
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
    
    ecmutex_lock (self->mutex);
    
    adbl_addPlugin (self, handle, moduleinfo->name);
    
    ecmutex_unlock (self->mutex);
  }
}

//----------------------------------------------------------------------------------------

void adbl_validate_config (AdblManager self, const EcString configpath, const EcString execpath)
{
  EcListNode node;
  
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
    EcList engines = eclist_create_ex (EC_ALLOC);
    
    eclogger_fmt (LL_TRACE, MODULE, "scan", "scan path '%s' for adbl modules", self->path);
    
    // fill a list with all files in that directory
    if (!ecdh_scan(self->path, engines, ENTC_FILETYPE_ISFILE))
    {
      eclogger_fmt (LL_ERROR, MODULE, "scan", "can't find path '%s'", ecstr_cstring(self->path) );
    }
    
    for (node = eclist_first(engines); node != eclist_end(engines); node = eclist_next(node))
    {
      EcString filename = eclist_data(node);
      // scan the library
      adbl_scanPlugin (self, filename);
      // clean up
      ecstr_delete(&filename);
    }
    // clean up
    eclist_free_ex (EC_ALLOC, &engines);
    
    if (self->observer)
    {
      adbl_parseConfig (self, configpath, FALSE);
    }
  }
  else
  {
    eclogger_msg (LL_ERROR, MODULE, "scan", "no scanpath defined in config");
  }
}

//----------------------------------------------------------------------------------------

void adbl_scanJsonFromFile (const EcString configpath, EcUdc* pdata)
{
  EcString filename = ecfs_mergeToPath (configpath, "adbl.json");
  
  eclogger_fmt (LL_TRACE, MODULE, "scan", "using config '%s'", filename);
  
  int res = ecjson_readFromFile (filename, pdata);
  if (res != ENTC_RESCODE_OK)
  {
    eclogger_fmt (LL_WARN, MODULE, "scan", "can't read json file '%s'", filename);
    return;
  }
  
  ecstr_delete(&filename);
}

//----------------------------------------------------------------------------------------

void adbl_scanJsonParse (AdblManager self, EcUdc data, const EcString configpath, const EcString execpath)
{
  ecstr_replace (&(self->path), ecudc_get_asString(data, "path", NULL));
  
  adbl_validate_config (self, configpath, execpath);
  
  EcUdc databases = ecudc_node(data, "databases");
  if (databases == NULL)
  {
    eclogger_fmt (LL_WARN, MODULE, "scan", "can't find databases in json");
    return;
  }
  
  void* cursor = NULL;
  EcUdc item;
  
  for (item = ecudc_next(databases, &cursor); item; item = ecudc_next(databases, &cursor))
  {
    const EcString dbname = ecudc_get_asString(item, "name", NULL);
    const EcString dbtype = ecudc_get_asString(item, "type", NULL);
    
    eclogger_fmt (LL_TRACE, MODULE, "scan", "found name = '%s', type = '%s'", dbname, dbtype);
    
    if (dbname && dbtype)
    {
      ADBLModuleProperties* pp;
      EcMapNode node;
      
      node = ecmap_find(self->modules, dbtype);
      if (node == ecmap_end (self->modules))
      {
        eclogger_fmt (LL_WARN, MODULE, "credentials", "database '%s' not in the list", dbtype);
        return;
      }
      
      pp = ecmap_data (node);
      
      node = ecmap_find(self->credentials, dbname);
      if( node != ecmap_end (self->credentials) )
      {
        eclogger_fmt (LL_WARN, MODULE, "credentials", "parsing the config file: db-source already exists [%s] in current register", dbname );
        continue;
      }
      
      {
        //create new credential
        AdblCredentials* pc = adbl_credentials_new (dbtype);
        pc->pp = pp;
        //add to map
        ecmap_append(self->credentials, dbname, pc);
        //parse the other stuff
        
        eclogger_fmt (LL_TRACE, MODULE, "scan", "added '%s'", dbname);
        
        pc->properties.host = ecstr_copy (ecudc_get_asString(item, "host", NULL));
        pc->properties.port = ecudc_get_asInt32 (item, "port", 0);
        
        pc->properties.username = ecstr_copy (ecudc_get_asString(item, "user", NULL));
        pc->properties.password = ecstr_copy (ecudc_get_asString(item, "pass", NULL));
        
        pc->properties.schema = ecstr_copy (ecudc_get_asString(item, "schema", NULL));
        
        EcUdc file = ecudc_node (item, "file");
        if (file)
        {
          const EcString fileprefix = ecudc_get_asString(item, "prefix", NULL);
          const EcString filextension = ecudc_get_asString(item, "ext", NULL);
          
          EcStream file = ecstream_new();
          
          if( fileprefix )
          {
            ecstream_append( file, fileprefix );
            ecstream_append( file, "_" );
          }
          
          if( pc->properties.schema )
          {
            ecstream_append( file, pc->properties.schema );
          }
          
          if( filextension )
          {
            ecstream_append( file, filextension );
          }
          else
          {
            ecstream_append( file, ".db" );
          }
          
          pc->properties.file = ecfs_mergeToPath (configpath, ecstream_buffer( file ));
          
          /* clean up */
          ecstream_delete(&file);
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

//----------------------------------------------------------------------------------------

void adbl_scan (AdblManager self, EcEventFiles events, const EcString configpath, const EcString execpath)
{
  if (isAssigned (self->observer))
  {
    ecf_observer_delete(self->observer);
  }
  
  eclogger_fmt (LL_TRACE, MODULE, "scan", "using configpath '%s'", configpath);        

  self->observer = ecf_observer_newFromPath (configpath, "adbl.xml", configpath, events, 0, 0);
  
  adbl_parseConfig (self, configpath, TRUE);  
  
  adbl_validate_config (self, configpath, execpath);
}

/*------------------------------------------------------------------------*/

AdblSession adbl_openSession (AdblManager self, const char* dbsource)
{
  /* variable definition */
  EcMapNode node;
  AdblCredentials* pc;
  AdblSession session;

  if (isNotAssigned(dbsource))
  {
    eclogger_msg (LL_WARN, MODULE, "session", "db-source was NULL" );
    return 0;  
  }
  
  node = ecmap_find(self->credentials, dbsource);
  if( node == ecmap_end(self->credentials) )
  {
    /* not found */
    eclogger_fmt (LL_WARN, MODULE, "session", "can't find db-source [%s] in current register", dbsource);        
    return 0;
  }

  ecmutex_lock(self->mutex);
  
  pc = ecmap_data(node);
      
  if (isNotAssigned(pc->connection))
  {
    /* not connected yet */
    /* try to connect */
    adbl_connect (self, pc);
    
    if (isNotAssigned(pc->connection))
    {
      ecmutex_unlock(self->mutex);
      return 0;
    }
  }
  /* get a new session from the plugin */
  ecmutex_unlock(self->mutex);

  session = ENTC_NEW(struct AdblSession_s);
  
  session->credentials = pc;
  
  return session;
}

/*------------------------------------------------------------------------*/

void adbl_closeSession (AdblSession* ptr)
{
  //AdblSession self = *ptr;
  
  ENTC_DEL(ptr, struct AdblSession_s);
}

/*------------------------------------------------------------------------*/

AdblCursor* adbl_dbquery (AdblSession session, AdblQuery* query, AdblSecurity* security)
{
  AdblCredentials* pc = session->credentials;
  void* cc;
  AdblCursor* cursor;

  if (isNotAssigned (pc->pp))
  {
    eclogger_msg (LL_ERROR, MODULE, "dbquery", "credentials without database" );
    return NULL;
  }
  
  if (isNotAssigned(pc->connection))
  {
    eclogger_msg (LL_WARN, MODULE, "dbquery", "no active database connection" );
    return 0;
  }
  
  if (isNotAssigned (pc->pp->dbquery))
  {
    eclogger_msg (LL_WARN, MODULE, "dbquery", "query method in matrix is not defined" );
    return 0;
  }
  
  /* check for faulty sql */
  adbl_query_sec (query, security);
      
  if( security->inicident )
  {
    return 0;  
  }

  cc = pc->pp->dbquery (pc->connection, query);
  if(!cc)
  {
    return 0;  
  }

  cursor = ENTC_NEW(struct AdblCursor_p);
  
  cursor->ptr = cc;
  cursor->pp = pc->pp;
  
  return cursor;
}

/*------------------------------------------------------------------------*/

int adbl_dbprocedure (AdblSession session, AdblProcedure* procedure, AdblSecurity* security)
{
  AdblCredentials* pc = session->credentials;

  if (isNotAssigned (pc->pp))
  {
    eclogger_msg (LL_ERROR, MODULE, "dbquery", "credentials without database" );
    return NULL;
  }

  if (isNotAssigned(pc->connection))
  {
    eclogger_msg (LL_WARN, MODULE, "dbquery", "no active database connection" );
    return 0;
  }

  if (isNotAssigned (pc->pp->dbprocedure))
  {
    eclogger_msg (LL_WARN, MODULE, "dbquery", "procedure method in matrix is not defined" );
    return 0;
  }

  // TODO
  
  /*
  if( security->inicident )
  {
    return 0;
  }
   */

  return pc->pp->dbprocedure (pc->connection, procedure);
}

/*------------------------------------------------------------------------*/

uint_t adbl_table_size (AdblSession session, const EcString table)
{
  AdblCredentials* pc = session->credentials;
  
  if (isNotAssigned (pc->pp))
  {
    eclogger_msg (LL_ERROR, MODULE, "size", "credentials without database" );
    return 0;
  }
  
  if (isAssigned(pc->connection))
  {
    if (isAssigned(pc->pp->dbtable_size))
    {
      return pc->pp->dbtable_size (pc->connection, table);
    }
  }
  return 0;
}

/*------------------------------------------------------------------------*/

AdblSequence* adbl_dbsequence_get( AdblSession session, const EcString table )
{
  AdblCredentials* pc = session->credentials;
  
  if (isNotAssigned (pc->pp))
  {
    eclogger_msg (LL_ERROR, MODULE, "sequence", "credentials without database" );
    return NULL;
  }
  
  if (isAssigned(pc->connection))
  {
    if (isAssigned(pc->pp->dbsequence_get))
    {
      void* ptr = pc->pp->dbsequence_get (pc->connection, table);
      
      if( ptr )
      {
        AdblSequence* sequence = ENTC_NEW(AdblSequence);
        
        sequence->pp = pc->pp;        
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
  AdblCredentials* pc = session->credentials;
  
  if (isNotAssigned (pc->pp))
  {
    eclogger_msg (LL_ERROR, MODULE, "update", "credentials without database" );
    return 0;
  }
  
  if (isNotAssigned(pc->connection))
  {
    eclogger_msg (LL_WARN, MODULE, "update", "no active database connection" );
    return 0;
  }
  
  if (isNotAssigned(pc->pp->dbquery))
  {
    eclogger_msg (LL_WARN, MODULE, "update", "update method in matrix is not defined" );
    return 0;
  }
  
  /* check for faulty sql */
  adbl_update_sec (update, security);
  
  if( security->inicident )
  {
    return 0;  
  }
  
  return pc->pp->dbupdate (pc->connection, update, isInsert);
}

/*------------------------------------------------------------------------*/

int adbl_dbinsert (AdblSession session, AdblInsert* insert, AdblSecurity* security)
{
  AdblCredentials* pc = session->credentials;
 
  if (isNotAssigned (pc->pp))
  {
    eclogger_msg (LL_ERROR, MODULE, "insert", "credentials without database" );
    return 0;
  }
  
  if (isNotAssigned(pc->connection))
  {
    eclogger_msg (LL_WARN, MODULE, "insert", "no active database connection" );
    return 0;
  }
  
  if (isNotAssigned(pc->pp->dbquery))
  {
    eclogger_msg (LL_WARN, MODULE, "insert", "insert method in matrix is not defined" );
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
    
  return pc->pp->dbinsert (pc->connection, insert);
}

/*------------------------------------------------------------------------*/

int adbl_dbdelete( AdblSession session, AdblDelete* del, AdblSecurity* security )
{
  AdblCredentials* pc = session->credentials;
  
  if (isNotAssigned (pc->pp))
  {
    eclogger_msg (LL_ERROR, MODULE, "delete", "credentials without database" );
    return 0;
  }
  
  if (isNotAssigned(pc->connection))
  {
    eclogger_msg (LL_WARN, MODULE, "delete", "no active database connection" );
    return 0;
  }
  
  if (isNotAssigned (pc->pp->dbquery))
  {
    eclogger_msg (LL_WARN, MODULE, "delete", "delete method in matrix is not defined" );
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
  
  return pc->pp->dbdelete (pc->connection, del);
}

/*------------------------------------------------------------------------*/

void adbl_dbbegin (AdblSession session)
{
  AdblCredentials* pc = session->credentials;
  
  if (isNotAssigned (pc->pp))
  {
    eclogger_msg (LL_ERROR, MODULE, "begin", "credentials without database" );
    return;
  }
  
  if (isAssigned (pc->connection))
  {
    if (isAssigned (pc->pp->dbbegin))
    {
      pc->pp->dbbegin(pc->connection);
    }
    else
    {
      eclogger_msg (LL_WARN, MODULE, "begin", "begin method in matrix is not defined" );
    }
  }
  else
  {
    eclogger_msg (LL_WARN, MODULE, "begin", "no active database connection" );
  }
}

/*------------------------------------------------------------------------*/

void adbl_dbcommit (AdblSession session)
{
  AdblCredentials* pc = session->credentials;
  
  if (isNotAssigned (pc->pp))
  {
    eclogger_msg (LL_ERROR, MODULE, "commit", "credentials without database" );
    return;
  }
  
  if (isAssigned (pc->connection))
  {
    if (isAssigned (pc->pp->dbcommit))
    {
      pc->pp->dbcommit (pc->connection);
    }
    else
    {
      eclogger_msg (LL_WARN, MODULE, "commit", "commit method in matrix is not defined" );
    }
  }
  else
  {
    eclogger_msg (LL_WARN, MODULE, "commit", "no active database connection" );
  }
}

/*------------------------------------------------------------------------*/

void adbl_dbrollback (AdblSession session)
{
  AdblCredentials* pc = session->credentials;
  
  if (isNotAssigned (pc->pp))
  {
    eclogger_msg (LL_ERROR, MODULE, "rollback", "credentials without database" );
    return;
  }
  
  if (isAssigned (pc->connection))
  {
    if (isAssigned (pc->pp->dbrollback))
    {
      pc->pp->dbrollback (pc->connection);
    }
    else
    {
      eclogger_msg (LL_WARN, MODULE, "commit", "rollback method in matrix is not defined" );
    }
  }
  else
  {
    eclogger_msg (LL_WARN, MODULE, "commit", "no active database connection" );
  }
}

/*------------------------------------------------------------------------*/

EcList adbl_dbschema (AdblSession session)
{
  AdblCredentials* pc = session->credentials;
  
  if (isNotAssigned (pc->pp))
  {
    eclogger_msg (LL_ERROR, MODULE, "schema", "credentials without database" );
    return NULL;
  }
  
  if (isAssigned (pc->connection))
  {
    if (isAssigned (pc->pp->dbrollback))
    {
      return pc->pp->dbschema (pc->connection);
    }
    else
    {
      eclogger_msg (LL_WARN, MODULE, "schema", "schema method in matrix is not defined" );
    }
  }
  else
  {
    eclogger_msg (LL_WARN, MODULE, "schema", "no active database connection" );
  }
  return NULL;
}

/*------------------------------------------------------------------------*/

AdblTable* adbl_dbtable (AdblSession session, const EcString tablename)
{
  AdblCredentials* pc = session->credentials;
  
  if (isNotAssigned (pc->pp))
  {
    eclogger_msg (LL_ERROR, MODULE, "table", "credentials without database" );
    return NULL;
  }
  
  if (isAssigned (pc->connection))
  {
    if (isAssigned (pc->pp->dbrollback))
    {
      return pc->pp->dbtable (pc->connection, tablename);
    }
    else
    {
      eclogger_msg (LL_WARN, MODULE, "table", "table method in matrix is not defined" );
    }
  }
  else
  {
    eclogger_msg (LL_WARN, MODULE, "table", "no active database connection" );
  }
  return NULL;  
}

/*------------------------------------------------------------------------*/

