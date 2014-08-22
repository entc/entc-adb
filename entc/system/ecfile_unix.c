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

#include <sys/stat.h>
#include <fcntl.h>

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

//--------------------------------------------------------------------------------

struct EcDirHandle_s
{

  DIR* dir;

  EcFileInfo_s node;

};

//--------------------------------------------------------------------------------

EcDirHandle ecdh_create (const EcString path)
{
  EcDirHandle self = NULL;
  
  DIR* dir = opendir (path);
  if (isAssigned (dir))
  {
    self = ENTC_NEW (struct EcDirHandle_s);
  
    self->dir = dir;
    self->node.name = ecstr_init ();
  }

  return self;
}

//--------------------------------------------------------------------------------

void ecdh_destroy (EcDirHandle* pself)
{
  EcDirHandle self = *pself;
  
  closedir (self->dir);
  
  ecstr_delete(&(self->node.name));
  
  ENTC_DEL (pself, struct EcDirHandle_s);
}

//--------------------------------------------------------------------------------

void ecdh_seekType (const EcString path, EcFileInfo entry)
{
  // unknown file type, this could happen on some file system types
  // for solving the issue call stat 
  // construct the file name 
  EcString inodename = ecfs_mergeToPath (path, entry->name);
    
  ecfs_fileInfo (entry, inodename);
    
  ecstr_delete (&inodename);
}

//--------------------------------------------------------------------------------

int ecdh_next (EcDirHandle self, EcFileInfo* pinfo)
{
  struct dirent* dentry = readdir (self->dir);

  if (isAssigned (dentry))
  {
    struct stat attribut;

    ecstr_replace(&(self->node.name), dentry->d_name);

    self->node.size = 0;
    
    stat (self->node.name, &attribut);
    
    self->node.cdate = attribut.st_ctime;
    self->node.mdate = attribut.st_mtime;
    self->node.adate = attribut.st_atime;
    
    if (dentry->d_type == DT_DIR)
    {
      self->node.type = ENTC_FILETYPE_ISDIR;
    }
    else if ((dentry->d_type == DT_REG) || (dentry->d_type == DT_LNK))
    {
      self->node.size = attribut.st_size;

      self->node.type = ENTC_FILETYPE_ISFILE;
    }
    else if (dentry->d_type == DT_UNKNOWN)
    {
      self->node.type = ENTC_FILETYPE_ISNONE;
    }
    else
    {
      self->node.type = ENTC_FILETYPE_ISNONE;      
    }
    
    *pinfo = &(self->node);
    
    return TRUE;
  }
  else
  {
    return FALSE;
  }
}

//--------------------------------------------------------------------------------

int ecfs_move (const EcString source, const EcString dest)
{
  return rename (source, dest) == 0;
}

//--------------------------------------------------------------------------------

int ecfs_mkdir (const EcString source)
{
  return mkdir (source, 0770) == 0;
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

int ecfs_fileInfo (EcFileInfo info, const EcString path)
{
  struct stat st;
  
  if( !path )
  {
    return FALSE;
  }
  
  if (stat (path, &st) != 0)
  {
    return FALSE;
  }
  
  if(S_ISREG(st.st_mode))
  {
    info->type = ENTC_FILETYPE_ISFILE;
  }
  else if(S_ISDIR(st.st_mode))
  {
    info->type = ENTC_FILETYPE_ISDIR;
  }
  else
  {
    info->type = ENTC_FILETYPE_ISNONE;              
  }
  
  return TRUE;
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

EcString ecfs_getExecutablePath (int argc, char *argv[])
{
  const EcString execPath = argv[0];
  return ecfs_getDirectory (execPath);
}

//-------------------------------------------------------------------------

#endif
