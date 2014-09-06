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

#if defined _WIN64 || defined _WIN32

#include "ecfile.h"
#include "../types/eclist.h"

#include <stdio.h>
#include <share.h>
#include <fcntl.h>
#include <io.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <shlwapi.h>
#pragma comment(lib, "shlwapi.lib")

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

  if( _sopen_s(&fd, filename, flags | _O_BINARY, _SH_DENYNO, _S_IREAD | _S_IWRITE) != 0 )
  {
    return 0;
  }
    
  fhandle = ENTC_NEW(struct EcFileHandle_s);
  
  fhandle->fd = fd;
  
  return fhandle;
}

/*------------------------------------------------------------------------*/

void ecfh_close(EcFileHandle* self)
{
  if( *self )
  {
    _close( (*self)->fd );
  }
  
  free(*self);
  
  *self = 0;
}

/*------------------------------------------------------------------------*/

void ecfh_writeString(EcFileHandle self, const EcString value)
{
  _write(self->fd, value, strlen(value));
}

/*------------------------------------------------------------------------*/

void ecfh_writeBuffer(EcFileHandle self, const EcBuffer buffer, uint_t size)
{
  _write(self->fd, buffer->buffer, size);
}

/*------------------------------------------------------------------------*/

void ecfh_writeConst(EcFileHandle self, const char* buffer, uint_t size)
{
  _write(self->fd, buffer, size);
}

/*------------------------------------------------------------------------*/

uint_t ecfh_readBuffer(EcFileHandle self, EcBuffer buffer)
{
  return _read(self->fd, buffer->buffer, buffer->size);  
}

/*------------------------------------------------------------------------*/

void ecfh_reset(EcFileHandle self)
{
  _lseek(self->fd, 0, FALSE);
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
  FILE* file;

  if( !self )
  {
    return 0;  
  }
  
  dub_handle = _dup(self->fd);

  file = _fdopen(dub_handle, type);

  return file;  
}

/*------------------------------------------------------------------------*/

EcString ecfh_md5(EcFileHandle self)
{
  ecfh_reset( self );

  
  
  ecfh_reset( self );

  return ecstr_copy("askjdhjsadbjavdjha");  
}

/*------------------------------------------------------------------------*/

#include <malloc.h>

struct EcDirHandle_s
{
  /* the handle */
  HANDLE dhandle;
  /* windows structure to store file informations */
  WIN32_FIND_DATA data;
  /* entc structure to store file informations */
  EcFileInfo_s info;

  int valid;
};

/*------------------------------------------------------------------------*/

EcDirHandle ecdh_create (const EcString path)
{
  /* variables */
  EcDirHandle self;
  EcString search_pattern;
  /* win32 types */
  WIN32_FIND_DATA ddata;
  HANDLE dhandle;

  if( !path )
  {
    return 0;
  }

  /* create a pattern to find all files */
  search_pattern = ecfs_mergeToPath(path, "*");
  /* should find the '.' of the folder */
  dhandle = FindFirstFile(search_pattern, &ddata );

  ecstr_delete(&search_pattern);

  if( dhandle == INVALID_HANDLE_VALUE )
  {
    return 0;
  }

  self = ENTC_NEW(struct EcDirHandle_s);
  /* we have a valid first entry */
  self->dhandle = dhandle;
  self->data = ddata;

  self->valid = TRUE;

  /* init */
  self->info.name = ecstr_init ();
  self->info.type = 0;

  return self;
};

/*------------------------------------------------------------------------*/

void ecdh_destroy (EcDirHandle* pself)
{
  if(!*pself) return;
  /* clsoe the handle */
  FindClose ((*pself)->dhandle);
  /* destroy the structures */
  ENTC_DEL(pself, struct EcDirHandle_s);
};

/*------------------------------------------------------------------------*/

int ecdh_next (EcDirHandle self, EcFileInfo* pinfo)
{
  if(!self) return FALSE;

  if( self->valid )
  {
    /* convert from windows infos to entc infos */
    ecstr_replace( &(self->info.name), self->data.cFileName);

    if (self->data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
    {
      self->info.type = ENTC_FILETYPE_ISDIR;
    }
    else
    {
      self->info.type = ENTC_FILETYPE_ISFILE;
    }

    self->info.size = (uint64_t) self->data.nFileSizeLow << 32 | self->data.nFileSizeHigh;

    self->info.cdate = self->data.ftCreationTime.dwLowDateTime;
    self->info.mdate = self->data.ftLastWriteTime.dwLowDateTime;
    self->info.adate = self->data.ftLastAccessTime.dwLowDateTime;
  
    self->valid = FindNextFile( self->dhandle, &(self->data) );

    *pinfo = &(self->info);
	  return TRUE;
  }
  return FALSE;
};

/*------------------------------------------------------------------------*/

void ecdh_seekType (const EcString path, EcFileInfo entry)
{
  // for windows not needed
}

/*------------------------------------------------------------------------*/

int ecfs_move(const EcString source, const EcString dest)
{
  return MoveFile(source, dest);
}

/*------------------------------------------------------------------------*/

int ecfs_mkdir(const EcString source)
{
  return CreateDirectory(source, NULL);
}

/*------------------------------------------------------------------------*/

int ecfs_rmdir(const EcString source)
{
  return RemoveDirectory(source);
}

/*------------------------------------------------------------------------*/

int ecfs_rmfile(const EcString source)
{
  return _unlink(source) == 0;
}

/*------------------------------------------------------------------------*/

time_t ecfs_filetime_to_timet(FILETIME ft)
{
  ULARGE_INTEGER ull;
  ull.LowPart = ft.dwLowDateTime;
  ull.HighPart = ft.dwHighDateTime;
  
  return ull.QuadPart / 10000000ULL - 11644473600ULL;
}

/*------------------------------------------------------------------------*/

int ecfs_fileInfo (EcFileInfo info, const EcString path)
{
  struct _stat64 st;

  if( !path )
  {
    return FALSE;
  }
  
  if (_stat64 (path, &st) != 0)
  {
    return FALSE;
  }

  info->name = ecstr_init ();
  info->mdate = st.st_mtime;

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

  {
    char mybuffer[MAX_PATH + 1];

    if( !PathCanonicalize(mybuffer, path) )
    {
      return 0;
    }

    buffer = ecstr_copy(mybuffer);
  }
  return buffer;
}

/*------------------------------------------------------------------------*/

EcString ecfs_getCurrentDirectory()
{
  return _getcwd( NULL, 0 );
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
    strncpy_s(resdir, diff01 + 1, file, diff01);
    // increase found
    found++;
    // allocate the memory for file part
    resfile = (char*)malloc( sizeof(char) * (strlen(found) + 1) );
    // copy file part
    strcpy_s(resfile, strlen(found) + 1, found);
  }
  else
  {
    resfile = ecstr_copy(file);
    resdir = ecstr_copy("");  
  }
  /* add basedir if not absolute */
  if( ((resdir)[0] != ENTC_PATH_SEPARATOR) && basedir )
  {
    ecstr_replace(&resdir, ecfs_mergeToPath(basedir, resdir));
  }  
  /* overrride the exported values */
  *ptr_resdir = resdir;
  *ptr_resfile = resfile;
}

//------------------------------------------------------------------------------------------------------------


EcString ecfs_getExecutablePath (int argc, char *argv[])
{
    char szFileName [MAX_PATH];
    GetModuleFileName (NULL, szFileName, MAX_PATH);

    return ecfs_getDirectory (szFileName);
}

//------------------------------------------------------------------------------------------------------------

#endif
