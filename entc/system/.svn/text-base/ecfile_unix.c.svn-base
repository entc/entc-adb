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

#ifdef __GNUC__

#include "ecfile.h"
#include "../types/eclist.h"

#include <stdio.h>

struct EcFileHandle_s
{
  int fd; 
};

/*------------------------------------------------------------------------*/

EcFileHandle ecfh_open(const EcString filename, int flags)
{
  /* variables */
  EcFileHandle fhandle;
  int fd;
  /* do some checks */
  if( !filename )
  {
    return 0;
  }

  fd = open(filename, flags, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);

  if( fd == -1 )
  {
    return 0;  
  }
    
  fhandle = ENTC_NEW(struct EcFileHandle_s);
  
  fhandle->fd = fd;
  
  return fhandle;
}

/*------------------------------------------------------------------------*/

void ecfh_close(EcFileHandle* pself)
{
  EcFileHandle self = *pself;
  
  close(self->fd);

  ENTC_DEL(pself, struct EcFileHandle_s);
}

/*------------------------------------------------------------------------*/

void ecfh_writeString(EcFileHandle self, const EcString value)
{
  write(self->fd, value, strlen(value));
}

/*------------------------------------------------------------------------*/

void ecfh_writeBuffer(EcFileHandle self, const EcBuffer buffer, uint_t size)
{
  write(self->fd, buffer->buffer, size);
}

/*------------------------------------------------------------------------*/

void ecfh_writeConst(EcFileHandle self, const char* buffer, uint_t size)
{
  write(self->fd, buffer, size);
}

/*------------------------------------------------------------------------*/

uint_t ecfh_readBuffer(EcFileHandle self, EcBuffer buffer)
{
  int res = read(self->fd, buffer->buffer, buffer->size);
  if (res < 0)
  {
    return 0;
  }
  return res;
}

/*------------------------------------------------------------------------*/

void ecfh_reset(EcFileHandle self)
{
  lseek(self->fd, 0, FALSE);
}

/*------------------------------------------------------------------------*/

int ecfh_fileno(EcFileHandle self)
{
  return self->fd;
}

/*------------------------------------------------------------------------*/

FILE* ecfh_file(EcFileHandle self, EcString type)
{
  /* variables */
  int dub_handle;

  if( !self )
  {
    return 0;  
  }
  
  dub_handle = dup(self->fd);
  if (dub_handle < 0)
  {
    return 0;
  }
  
  return fdopen(dub_handle, type);
}

/*------------------------------------------------------------------------*/

EcString ecfh_md5(EcFileHandle self)
{
  ecfh_reset( self );

    
  ecfh_reset( self );

  return ecstr_copy("askjdhjsadbjavdjha");  
}

/*------------------------------------------------------------------------*/

EcDirHandle ecdh_new(const EcString path)
{
  if( path )
  {
    return opendir(path);
  }
  else
  {
    return 0;  
  }
}

/*------------------------------------------------------------------------*/

void ecdh_close(EcDirHandle* self)
{
  closedir( *self );
  
  self = 0;
}

/*------------------------------------------------------------------------*/

int ecdh_next(EcDirHandle self, EcDirNode* pnode)
{
  *pnode = readdir (self);
  return isAssigned(*pnode);
}

/*------------------------------------------------------------------------*/

int ecdh_getFileType(const EcString path, EcDirNode entry)
{
  if( entry->d_type == DT_DIR )
  {
    return ENTC_FILETYPE_ISDIR;
  }
  else if( ( entry->d_type == DT_REG ) || ( entry->d_type == DT_LNK ) )
  {
    return ENTC_FILETYPE_ISFILE;
  }
  else if( entry->d_type == DT_UNKNOWN )
  {
    /* unknown file type, this could happen on some file system types */
    /* for solving the issue call stat */
    EcStatInfo st;
    /* construct the file name */
    EcString inodename = ecfs_mergeToPath(path, entry->d_name);
    
    ecfs_stat(&st, inodename);
    
    ecstr_delete (&inodename);
    
    if(S_ISREG(st.st_mode))
    {
      return ENTC_FILETYPE_ISFILE;
    }
    else if(S_ISDIR(st.st_mode))
    {
      return ENTC_FILETYPE_ISDIR;
    }
    else
    {
      return ENTC_FILETYPE_ISNONE;              
    }
  }
  return ENTC_FILETYPE_ISNONE;
}

/*------------------------------------------------------------------------*/

int ecfs_move(const EcString source, const EcString dest)
{
  return rename(source, dest) == 0;
}

/*------------------------------------------------------------------------*/

int ecfs_mkdir(const EcString source)
{
  return mkdir(source, 0770) == 0;
}

/*------------------------------------------------------------------------*/

int ecfs_rmdir(const EcString source)
{
  return unlink( source ) == 0;
}

/*------------------------------------------------------------------------*/

int ecfs_rmfile(const EcString source)
{
  return unlink( source ) == 0;
}

/*------------------------------------------------------------------------*/

int ecfs_stat(EcStatInfo* info, const EcString path)
{
  if( !path )
  {
    return FALSE;
  }
  
  return stat( path, info ) == 0;
}

/*------------------------------------------------------------------------*/

char* ecfs_getRealPath(const EcString path)
{
  /* variables */
  char* buffer;

  if( !path )
  {
    return 0;
  }

#ifdef PATH_MAX
  {
    char mybuffer[PATH_MAX + 1];
    /* use a static buffer... FIXME */
    buffer = ecstr_copy( realpath(path, mybuffer) );
  }
#else
  /* buffer of the length 400 */
  buffer = (char*)malloc( sizeof(char) * 1024 );
  
  if( !realpath(path, buffer) )
  {
    free(buffer);
    
    return 0;
  }
#endif

  return buffer;
}

/*------------------------------------------------------------------------*/

EcString ecfs_getCurrentDirectory()
{
  /* define a static char */
  char cpath[1025];

  return ecstr_copy( getcwd( cpath, 1024 ) );
}

/*------------------------------------------------------------------------*/

void ecfs_basedir(const EcString basedir, const EcString file, EcString* ptr_resdir, EcString* ptr_resfile)
{
  /* get the chars */
  char* resdir = 0;
  char* resfile = 0;
  char* found = 0;
  
  /* check if file is valid */
  if( !file )
    return;
  
  /* parse file to get dir and file */
  found = strrchr(file, ENTC_PATH_SEPARATOR);
  if( found && *(found + 1) )
  {
    // find the difference between the found and original
    uint_t diff01 = found - file;
    // allocate some memory
    resdir = (char*)malloc( sizeof(char) * (diff01 + 1) );
    // copy the dir part
    strncpy(resdir, file, diff01);
    // increase found
    found++;
    // allocate the memory for file part
    resfile = (char*)malloc( sizeof(char) * (strlen(found) + 1) );
    // copy file part
    strcpy(resfile, found);
  }
  else
  {
    resfile = ecstr_copy(file);
    resdir = ecstr_copy("");  
  }
  /* add basedir if not absolute */
  if( ((resdir)[0] != ENTC_PATH_SEPARATOR) && basedir )
  {
    ecstr_replaceTO(&resdir, ecfs_mergeToPath(basedir, resdir));
  }  
  /* overrride the exported values */
  *ptr_resdir = resdir;
  *ptr_resfile = resfile;
}

/*------------------------------------------------------------------------*/

#endif
