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

#ifdef __DOS__

#include "qcfile.h"

#include "../types/qclist.h"
#include "../qsecfile.h"

#include <stdio.h>
#include <dos.h>
#include <string.h>

struct QCFileHandle_s
{
  int fd; 
};


struct QCDirHandle_s {

  struct find_t dh;
  
  int valid;
  
  struct QCDirNode_s node;

};

/*------------------------------------------------------------------------*/

QCFileHandle qcfh_open(const QCString filename, int flags)
{
  /* variables */
  QCFileHandle fhandle;
  int fd;
  /* do some checks */
  if( !filename )
  {
    return 0;
  }

  if (_dos_open(filename, _S_IREAD | _S_IWRITE, &fd) )
  {
    return 0;
  }
    
  fhandle = QNEW(struct QCFileHandle_s);
  
  fhandle->fd = fd;
  
  return fhandle;  
}

/*------------------------------------------------------------------------*/

void qcfh_close(QCFileHandle* self)
{
  if( *self )
  {
    _dos_close( (*self)->fd );
  }
  
  QDEL(self, struct QCFileHandle_s);
}

/*------------------------------------------------------------------------*/

void qcfh_writeString(QCFileHandle self, const QCString value)
{
  unsigned res;
  _dos_write(self->fd, value, strlen(value), &res);
}

/*------------------------------------------------------------------------*/

void qcfh_writeBuffer(QCFileHandle self, const QCBuffer buffer, uint_t size)
{
  unsigned res;
  _dos_write(self->fd, buffer->buffer, size, &res);
}

/*------------------------------------------------------------------------*/

void qcfh_writeConst(QCFileHandle self, const char* buffer, uint_t size)
{
  unsigned res;
  _dos_write(self->fd, buffer, size, &res);
}

/*------------------------------------------------------------------------*/

uint_t qcfh_readBuffer(QCFileHandle self, QCBuffer buffer)
{
  unsigned res;
  
  _dos_read(self->fd, buffer->buffer, buffer->size, &res);

  return res;
}

/*------------------------------------------------------------------------*/

void qcfh_reset(QCFileHandle self)
{
  // this is unknown in dos ????
  
  //DOS_LSEEK(self->fd, 0, FALSE);
}

/*------------------------------------------------------------------------*/

int qcfh_fileno(QCFileHandle self)
{
  return self->fd;
}

/*------------------------------------------------------------------------*/

FILE* qcfh_file(QCFileHandle self, QCString type)
{
  /* variables */
  int dub_handle;
  FILE* file;

  if( !self )
  {
    return 0;  
  }
  
  dub_handle = dup(self->fd);
  file = fdopen(dub_handle, type);

  return file;  
}

/*------------------------------------------------------------------------*/

QCString qcfh_md5(QCFileHandle self)
{
  qcfh_reset( self );

  qcfh_reset( self );

  return qcstr_copy("askjdhjsadbjavdjha");  
}

/*------------------------------------------------------------------------*/

QCDirHandle qcdh_new(const QCString path)
{
  /* variables */
  QCDirHandle self;
  QCString search_pattern;
  /* win32 types */
  struct find_t dhandle;
  int ret;

  if( !path )
  {
    return 0;
  }
  
  /* create a pattern to find all files */
  search_pattern = qcfs_mergeToPath(path, "*");
  
  ret = _dos_findfirst( search_pattern, _A_NORMAL | _A_RDONLY | _A_SUBDIR | _A_HIDDEN, &dhandle);
  
  qcstr_delete(&search_pattern);
  
  /* should find the '.' of the folder */
  if (ret != 0)
  {
    return 0;
  }

  self = QNEW(struct QCDirHandle_s);
  /* we have a valid first entry */
  self->dh = dhandle;
  self->valid = TRUE;
  
  /* init */
  self->node.d_name = 0;
  self->node.d_type = 0;

  return self;
};

/*------------------------------------------------------------------------*/

void qcdh_close(QCDirHandle* pself)
{
  QCDirHandle self = *pself;  
  /* clsoe the handle */
  _dos_findclose(&(self->dh));
  /* destroy the structures */
  QDEL(pself, struct QCDirHandle_s);
};

/*------------------------------------------------------------------------*/

QCDirNode qcdh_next(QCDirHandle self)
{
  if(!self) return 0;
  
  if( self->valid )
  {
    /* convert from windows infos to entc infos */
    qcstr_replace( &(self->node.d_name), self->dh.name);
    self->node.d_type = self->dh.attrib;
    
    self->valid = _dos_findnext(&(self->dh)) == 0;
    
    return &(self->node);
  }
  return 0;
};

/*------------------------------------------------------------------------*/

int qcdh_getFileType(const EcString path, EcDirNode entry)
{
  if( entry->d_type & _A_SUBDIR )
  {
    return Ec_FILETYPE_ISDIR;
  }
  return QC_FILETYPE_ISFILE;
}

/*------------------------------------------------------------------------*/

int qcfs_move(const QCString source, const QCString dest)
{
  return rename(source, dest) == 0;
}

/*------------------------------------------------------------------------*/

int qcfs_mkdir(const QCString source)
{
  return mkdir(source) == 0;
}

/*------------------------------------------------------------------------*/

int qcfs_rmdir(const QCString source)
{
  return unlink( source ) == 0;
}

/*------------------------------------------------------------------------*/

int qcfs_rmfile(const QCString source)
{
  return unlink(source) == 0;
}

/*------------------------------------------------------------------------*/

int qcfs_stat(QCStatInfo* info, const QCString path)
{
  return _dos_findfirst( path, _A_NORMAL | _A_RDONLY | _A_SUBDIR | _A_HIDDEN, info) == 0;
}

/*------------------------------------------------------------------------*/

char* qcfs_getRealPath(const QCString path)
{
  return qcstr_copy(path);
}

/*------------------------------------------------------------------------*/

QCString qcfs_getCurrentDirectory()
{
  return getcwd( NULL, 0 );
}

/*------------------------------------------------------------------------*/

void qcfs_basedir(const QCString basedir, const QCString file, QCString* ptr_resdir, QCString* ptr_resfile)
{
  /* get the chars */
  char* resdir = 0;
  char* resfile = 0;
  char* found = 0;
  
  /* check if file is valid */
  if( !file )
    return;
  
  /* parse file to get dir and file */
  found = strrchr(file, QC_PATH_SEPARATOR);
  if( found && *(found + 1) )
  {
    // find the difference between the found and original
    uint_t diff01 = found - file;
    // allocate some memory
    resdir = (char*)QMALLOC( sizeof(char) * (diff01 + 1) );
    // copy the dir part
    strncpy(resdir, file, diff01);
    // increase found
    found++;
    // allocate the memory for file part
    resfile = (char*)QMALLOC( sizeof(char) * (strlen(found) + 1) );
    // copy file part
    strcpy(resfile, found);
  }
  else
  {
    resfile = qcstr_copy(file);
    resdir = qcstr_copy("");  
  }
  /* add basedir if not absolute */
  if( ((resdir)[0] != QC_PATH_SEPARATOR) && basedir )
  {
    qcstr_replace(&resdir, qcfs_mergeToPath(basedir, resdir));
  }  
  /* overrride the exported values */
  *ptr_resdir = resdir;
  *ptr_resfile = resfile;
}

#endif
