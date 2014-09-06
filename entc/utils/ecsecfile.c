/*
 * Copyright (c) 2010-2013 "Alexander Kalkhof" [email:entc@kalkhof.org]
 *
 * This file is part of the extension n' tools (entc-base) framework for C.
 *
 * entc-base is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * entc-base is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with entc-base.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "ecsecfile.h"
#include "utils/eclogger.h"

/* c incudes */
#ifdef _WIN32
#include <io.h>
#include <share.h>
#endif

#include <fcntl.h>
#include <sys/stat.h>

#include <string.h>
#include <errno.h>

/* quom core includes */
#include "types/ecstring.h"

int ecsec_file_checkpath(const EcString path, const EcString confdir, EcLogger logger)
{
  const char* c1 = confdir;
  const char* c2 = path;
  while( *c1 )
  {
    if( *c1 != *c2 )
    {
      // report security breach level yellow
      eclogger_sec (logger, SL_YELLOW);
      
      eclogger_logformat(logger, LL_INFO, "QSEC", "access to path '%s' outside confdir '%s'", path, confdir);

      return FALSE;  
    }
    
    c1++;
    c2++;
  }
  return TRUE;
}

/*------------------------------------------------------------------------*/

int ecsec_checkPath(const EcString path, const EcString confdir, EcLogger logger)
{
  /* check if the found path is indside the confdir */
  if( !ecsec_file_checkpath(path, confdir, logger) )
  {
    if( strcmp(path, "/etc") == 0 )
    {
      // report security breach level red
      eclogger_sec (logger, SL_RED);
      
      eclogger_logformat (logger, LL_INFO, "QSEC", "tried to access /etc with file '%s'", path );      

      return 2;
    }
    
    return 1;
  }
  return 0;
}

/*------------------------------------------------------------------------*/

int ecsec_extractPathAndCheck(struct EcSecFopen* inst, EcLogger logger, const EcString confdir)
{
  /* variables */
  char* find = 0;
  char* filename_copy = ecstr_copy( inst->filename );
  /* find last occurence of QPATH_SEPARATOR */
  find = strrchr( filename_copy, ENTC_PATH_SEPARATOR );
  if( !find )
  {
    /* no path found for the file */
    /* merge confdir and file */
    filename_copy = ecstr_replaceTO(&filename_copy, ecfs_mergeToPath(confdir, filename_copy));
    
    find = strrchr( filename_copy, ENTC_PATH_SEPARATOR );
    
    if(!find)
    {
      eclogger_logformat(logger, LL_ERROR, "QSEC", "no valid path or file '%s'", filename_copy);
      // clean up
      ecstr_delete( &filename_copy );
      return FALSE;
    }
    /* the file seems to have no separator */
  }
  /* override separator with null */
  *find = 0;
  
  inst->sec_error = ecsec_checkPath(filename_copy, confdir, logger);
  
  ecstr_delete( &filename_copy );

  return inst->sec_error == 0;
}

/*------------------------------------------------------------------------*/

int ecsec_fopen(struct EcSecFopen* self, const EcString filename, int flags, EcLogger logger, const EcString confdir)
{
  /* variables */
  EcFileInfo_s info;
  /* init the structure */
  self->fhandle = 0;
  self->os_error = 0;
  self->sec_error = 0;
  self->mtime = 0;
  self->size = 0;
  self->filename = 0;
  /* check if the file exists */  
  if (ecfs_fileInfo(&info, filename))
  {
    /* copy attributes from stat */
    self->mtime = info.mdate;
    self->size = info.size;
    /* get first the real path from the system */
    self->filename = ecfs_getRealPath(filename);
    
    if( !(self->filename) )
    {     
      eclogger_logerrno(logger, LL_ERROR, "QSEC", "can't get correct realpath for '%s'", ecstr_cstring(filename));
      
      return FALSE;
    }
  }
  else
  {
    /* file doesn't exists */
    if( flags & O_WRONLY )
    {
      self->filename = ecstr_copy( filename );
      /* go on */
    }
    else
    {
      eclogger_logerrno(logger, LL_ERROR, "QSEC", "can't open '%s'", ecstr_cstring(filename));
      
	     return FALSE;
    }
  }
  
  if( !ecsec_extractPathAndCheck(self, logger, confdir) )
  {
    ecstr_delete( &(self->filename) );
    
    return FALSE;
  }
  /* now all checks are done, open the file */
  {  
    self->fhandle = ecfh_open(self->filename, flags );
  }

  if( !self->fhandle )
  {
    eclogger_logerrno(logger, LL_ERROR, "QSEC", "can't open the file '%s'", self->filename);
    
    self->os_error = errno;
    
    //      eclogger_logerrno(logger, LOGMSG_SECERROR, "QSEC", inst->os_error, "can't open the file '%s'", inst->filename);

    ecstr_delete( &(self->filename) );

    return FALSE;
  }
  
  return TRUE;
}

/*------------------------------------------------------------------------*/

int ecsec_dopen(struct EcSecDopen* inst, const EcString path, EcLogger logger, const EcString confdir)
{
  /* variables */
  EcFileInfo_s info;
  /* init the structure */
  inst->dh = 0;
  inst->mtime = 0;
  /* check if the file exists */  
  if (ecfs_fileInfo (&info, path))
  {
    inst->mtime = info.mdate;
    /* get first the real path from the system */
    inst->path = ecfs_getRealPath(path);
    
    if( !inst->path )
    {      
      eclogger_logerrno(logger, LL_ERROR, "QSEC", "can't get correct realpath for '%s'", ecstr_cstring(path));
      
      return FALSE;
    }
  }
  else
  {
    inst->path = ecstr_copy( path );
    
    eclogger_logerrno(logger, LL_ERROR, "QSEC", "can't open '%s'", ecstr_cstring(path));
    
	   return FALSE;
  }  
  /* now check the path */
  inst->sec_error = ecsec_checkPath(inst->path, confdir, logger);

  if( inst->sec_error == 0 )
  {
    inst->dh = ecdh_create (inst->path);

    return TRUE;
  }

  return FALSE;
}

/*------------------------------------------------------------------------*/

void ecsec_mkdir(const EcString path, EcLogger logger, const EcString confdir)
{
  // variables
  int res = 0;
  EcFileInfo_s info;
  // does the path exists
  if (ecfs_fileInfo (&info, path))
  {
    // file exists
    return;
  }
  // try to create the path
#ifdef ENTC_PLATFORM_WINDOWS
  res = _mkdir (path);
#else
  res = mkdir (path, 0770);
#endif
  
  if (res != 0)
  {
    eclogger_logerrno(logger, LL_ERROR, "CORE", "create path '%s'", path );    
  }  
}

/*------------------------------------------------------------------------*/
