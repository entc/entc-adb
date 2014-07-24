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

#include "ecobserver.h"

/* quom core includes */
#include "../system/ecmutex.h"
#include "../types/eclist.h"
#include "../utils/ecsecfile.h"

#include <fcntl.h>

#if defined _WIN64 || defined _WIN32
#include <windows.h>
#endif

/*------------------------------------------------------------------------*/

/* for MacOSX */
#ifdef __APPLE_CC__

#include <sys/event.h>

#elif __linux__

#include <sys/inotify.h>

#endif

/*------------------------------------------------------------------------*/

struct EcFileObserver_s
{

  EcString filename;
  
  EcString confdir;

  EcFileHandle fhandle;

  events_callback_fct fct;
  
  void* ptr;

  /* reference to the logger */
  EcLogger logger;
  /* reference to the event manager */  
  EcEventFiles events;
    
};

/*------------------------------------------------------------------------*/

void ecf_observer_onDelete(void* ptr)
{
  /* variables */
  struct EcSecFopen secopen;
  int counter = 0;
  
  EcFileObserver self = ptr;
  
  eclogger_log(self->logger, LL_DEBUG, "CORE", "{fobserver} Received onDelete event" );

  ecfh_close( &(self->fhandle) );
  
  while ( !ecsec_fopen(&secopen, self->filename, O_RDONLY, self->logger, self->confdir) )
  {
    /* fixme export this into an extra thread */
#ifdef _WIN32
    Sleep(1);
#else
    sleep(1);
#endif

    counter++;
    
    if( counter > 5 )
    {
      return;  
    }
  }
    
  self->fhandle = secopen.fhandle;
  
#ifdef __APPLE_CC__
  /* reestablish the kevent handler */
  ece_files_register(self->events, ecfh_fileno(self->fhandle), EVFILT_VNODE, NOTE_WRITE, self->fct, self->ptr, ecf_observer_onDelete, self);

#elif __linux__
  
  ece_files_register(self->events, self->filename, self->fct, self->ptr, ecf_observer_onDelete, self);
  
#else
  
  ece_files_register (self->events, self->filename, self->fct, self->ptr, ecf_observer_onDelete, self);
  
#endif
 
  if( self->fct )
  {
    self->fct( self->ptr );  
  }
}

/*------------------------------------------------------------------------*/

EcFileObserver ecf_observer_newFromPath(const EcString path, const EcString filename, const EcString confdir, EcEventFiles events, EcLogger logger, events_callback_fct fct, void* ptr)
{
  EcString lrealpath = ecfs_mergeToPath(path, filename);
  
  EcFileObserver inst = ecf_observer_new(lrealpath, confdir, events, logger, fct, ptr);
  
  ecstr_delete(&lrealpath);
  
  return inst;  
}

/*------------------------------------------------------------------------*/

EcFileObserver ecf_observer_new(const EcString filename, const EcString confdir, EcEventFiles events, EcLogger logger, events_callback_fct fct, void* ptr)
{
  EcFileObserver self = ENTC_NEW(struct EcFileObserver_s);
  /* init common part */
  self->confdir = ecstr_copy(confdir);
  self->filename = ecstr_copy( filename );

  self->fhandle = 0;
  
  self->fct = fct;
  self->ptr = ptr;
    
  self->logger = logger;
  self->events = events;

#ifdef _WIN32
  ece_files_register (self->events, self->filename, self->fct, self->ptr, ecf_observer_onDelete, self);
#endif

  return self;
}

/*------------------------------------------------------------------------*/

void ecf_observer_delete(EcFileObserver self)
{
  if (self->fhandle)
  {
    ecfh_close( &(self->fhandle) );
  }
  
  ecstr_delete( &(self->filename) );
  ecstr_delete( &(self->confdir) );
  
  ENTC_DEL( &self, struct EcFileObserver_s );
}

/*------------------------------------------------------------------------*/

EcFileHandle ecf_observer_open(EcFileObserver self)
{
  /* variables */
  struct EcSecFopen secopen;
  
  if( !(self->filename) )
  {
    return 0;  
  }
  
  if( self->fhandle )
  {
    ecfh_reset( self->fhandle );
    /* already open */
    return self->fhandle;
  }
  
  if( !self->events )
  {
    eclogger_log(self->logger, LL_WARN, "CORE", "{fo::open} EventManager missing" );
    
    return 0;  
  }
  
  if( !ecsec_fopen(&secopen, self->filename, O_RDONLY, self->logger, self->confdir) )
  {
    return 0;
  }
  
  self->fhandle = secopen.fhandle;
  
  ecstr_replaceTO( &(self->filename) , secopen.filename );
  
#ifdef __APPLE_CC__

  ece_files_register(self->events, ecfh_fileno(self->fhandle), EVFILT_VNODE, NOTE_WRITE, self->fct, self->ptr, ecf_observer_onDelete, self);

#elif __linux__

  ece_files_register(self->events, self->filename, self->fct, self->ptr, ecf_observer_onDelete, self);

#else

  
#endif
  
  return secopen.fhandle;
}

/*------------------------------------------------------------------------*/

void ecf_observer_close(EcFileObserver self)
{
#ifdef __APPLE_CC__
  /* don't close the file handle */
#elif __linux__
  /* don't close the file handle */
#else
  ecfh_close( &(self->fhandle) );
#endif
}

/*------------------------------------------------------------------------*/
    
int ecf_observer_exists(EcFileObserver self)
{
  return ecstr_valid(self->filename);
}
    
/*------------------------------------------------------------------------*/
    
const EcString ecf_observer_getFileName(EcFileObserver self)
{
  return self->filename;  
}
    
/*------------------------------------------------------------------------*/
    
struct EcDirObserver_s
{
  /* reference to the logger */
  EcLogger logger;
  /* reference to the event manager */  
  EcEventFiles events;
  
  EcDirHandle dh;
  
  EcString path;
  
  /* was event registered */
  int event;
  
  events_callback_fct fct;
  
  void* ptr;
  
};

/*------------------------------------------------------------------------*/

EcDirObserver ecd_observer_new(const EcString path, EcEventFiles events, EcLogger logger)
{
  EcDirObserver inst = ENTC_NEW(struct EcDirObserver_s);
  /* init common part */
  inst->logger = logger;
  inst->events = events;
  inst->dh = 0;
  inst->path = ecfs_getRealPath( path );
  if( !inst->path )
  {
    eclogger_logerrno(inst->logger, LL_ERROR, "CORE", "DirObserver can't open path '%s'", ecstr_cstring(path));
  }
  /* set the flag first to false */
  inst->event = 0;
  
  return inst;  
}

/*------------------------------------------------------------------------*/

void ecd_observer_delete(EcDirObserver inst)
{
  ecd_observer_close( inst );
  
#ifdef __APPLE_CC__
  if( inst->dh )
  {
    ecdh_destroy (&(inst->dh));    
  }
#elif __linux__
  if( inst->dh )
  {
    ecdh_destroy (&(inst->dh));    
  }
#endif
  
  ecstr_delete( &(inst->path) );
  
  free( inst );  
}

/*------------------------------------------------------------------------*/

EcDirHandle ecd_observer_open(EcDirObserver self, const EcString confdir)
{
  /* variables */
  struct EcSecDopen secopen;
  
  if( !self->path )
  {
    return 0;  
  }
  
  if( self->dh )
  {
    /* already open */
    return self->dh;
  }
  
  if( !ecsec_dopen(&secopen, self->path, self->logger, confdir) )
  {
    eclogger_logformat(self->logger, LL_ERROR, "CORE", "DirObserver can't open path '%s'", ecstr_cstring(secopen.path) );
    
    return 0;
  }
  
  self->dh = secopen.dh;
  ecstr_replaceTO( &(self->path), secopen.path);
  
#ifdef __APPLE_CC__

 // ecevents_register(self->events, self->dh, EVFILT_VNODE, (NOTE_DELETE | NOTE_EXTEND | NOTE_WRITE | NOTE_ATTRIB | NOTE_RENAME), self->fct, self->ptr);
  
#elif __linux__
  
#else
  
#endif    
  
  return secopen.dh;
}

/*------------------------------------------------------------------------*/

void ecd_observer_close(EcDirObserver inst)
{
#ifdef __APPLE_CC__
  /* don't close the file handle */
#elif __linux__
  /* don't close the file handle */
#else
  if( inst->dh )
  {
    ecdh_close( &(inst->dh) );    
  }
#endif
}

/*------------------------------------------------------------------------*/

const EcString ecd_observer_getDirName(EcDirObserver inst)
{
  return inst->path;
}

/*------------------------------------------------------------------------*/
