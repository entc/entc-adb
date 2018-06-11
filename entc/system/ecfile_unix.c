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

#include "ecfile.h"

#include "system/macros.h"

#if defined __BSD_OS || defined __LINUX_OS

#include "system/ecdefs.h"
#include "types/eclist.h"
#include "tools/eclog.h"

#if defined __APPLE__

#include <limits.h>
#include <copyfile.h>
#include <mach-o/dyld.h>

#elif defined __BSD_OS



#elif defined __LINUX_OS

#include <sys/sendfile.h>

#endif

#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>

#include <openssl/md5.h>

struct EcFileHandle_s
{
  int fd; 
};

/*------------------------------------------------------------------------*/

EcFileHandle ecfh_open (const EcString filename, int flags)
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

uint64_t ecfh_size (EcFileHandle self)
{
  struct stat st;
  
  fstat (self->fd, &st);
  
  return st.st_size;
}

/*------------------------------------------------------------------------*/

int ecfh_writeString(EcFileHandle self, const EcString value)
{
  return write (self->fd, value, strlen(value));
}

/*------------------------------------------------------------------------*/

int ecfh_writeBuffer (EcFileHandle self, const EcBuffer buffer, uint_t size)
{
  return write (self->fd, buffer->buffer, size);
}

/*------------------------------------------------------------------------*/

int ecfh_writeConst(EcFileHandle self, const char* buffer, uint_t size)
{
  return write (self->fd, buffer, size);
}

/*------------------------------------------------------------------------*/

uint_t ecfh_readBuffer (EcFileHandle self, EcBuffer buffer)
{
  int res = read(self->fd, buffer->buffer, buffer->size);
  if (res < 0)
  {
    return 0;
  }
  return res;
}

//--------------------------------------------------------------------------------

uint_t ecfh_readBufferOf (EcFileHandle self, EcBuffer buffer, uint_t offset)
{
  int res = read (self->fd, buffer->buffer + offset, buffer->size - offset);
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


EcString ecfh_md5 (EcFileHandle self)
{
  EcBuffer buf = ecbuf_create (1024);
  EcBuffer ret = ecbuf_create (32);
  int bytes;
  MD5_CTX mdContext;
  unsigned char c[MD5_DIGEST_LENGTH];
  int i = 0;
  
  ecfh_reset( self );

  // init the MD5 content
  MD5_Init (&mdContext);
  
  while ((bytes = read (self->fd, buf->buffer, buf->size)) != 0)
  {
    MD5_Update (&mdContext, buf->buffer, bytes);
  }
  
  MD5_Final (c, &mdContext);
  
  ecfh_reset( self );

  for (i = 0; i < MD5_DIGEST_LENGTH; i++)
  {
    snprintf((char*)(ret->buffer) + (i * 2), 4, "%02x", c[i]);
  }

  ecbuf_destroy (&buf);
  
  return ecbuf_str(&ret);
}

//--------------------------------------------------------------------------------

struct EcDirHandle_s
{

  DIR* dir;

  EcFileInfo_s node;
  
  EcString path;

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
    self->path = ecstr_copy (path);
  }

  return self;
}

//--------------------------------------------------------------------------------

void ecdh_destroy (EcDirHandle* pself)
{
  EcDirHandle self = *pself;
  
  closedir (self->dir);
  
  ecstr_delete(&(self->node.name));
  ecstr_delete(&(self->path));
  
  ENTC_DEL (pself, struct EcDirHandle_s);
}

//--------------------------------------------------------------------------------

const EcString ecdh_path (EcDirHandle self)
{
  return self->path;
}

//--------------------------------------------------------------------------------

void ecdh_seekType (const EcString path, EcFileInfo entry)
{
  // unknown file type, this could happen on some file system types
  // for solving the issue call stat 
  // construct the file name 
  EcString inodename = ecfs_mergeToPath (path, entry->name);
    
  if (ecfs_fileInfo (entry, inodename))
  {
    
  }
  else
  {
    
  }
    
  ecstr_delete (&inodename);
}

//--------------------------------------------------------------------------------

int ecdh_next_full (EcDirHandle self, EcFileInfo* pinfo, struct dirent* dentry)
{
  // create the full path
  EcString filename = ecfs_mergeToPath (self->path, dentry->d_name);
  
  int res = ecfs_fileInfo (&(self->node), filename);
  
  ecstr_delete (&filename);
  
  if (!res)
  {
    return FALSE;
  }
  
  return TRUE;
}

//--------------------------------------------------------------------------------

int ecdh_next_simple (EcDirHandle self, EcFileInfo* pinfo, struct dirent* dentry)
{
  if (dentry->d_type == DT_DIR)
  {
    self->node.type = ENTC_FILETYPE_ISDIR;
  }
  else if ((dentry->d_type == DT_REG) || (dentry->d_type == DT_LNK))
  {
    self->node.type = ENTC_FILETYPE_ISFILE;
  }

  return ecdh_next_full (self, pinfo, dentry);
}

//--------------------------------------------------------------------------------

int ecdh_next (EcDirHandle self, EcFileInfo* pinfo, int fullInfo)
{
  struct dirent* dentry = readdir (self->dir);
  
  if (isAssigned (dentry))
  {
    *pinfo = &(self->node);
    
    ecstr_replace (&(self->node.name), dentry->d_name);              
    
    if (fullInfo)
    {
      return ecdh_next_full (self, pinfo, dentry);
    }
    else 
    {
      return ecdh_next_simple (self, pinfo, dentry);
    }
  }
  else
  {
    return FALSE;
  }
}

//--------------------------------------------------------------------------------

int ecfs_move (const EcString source, const EcString dest)
{
  if (rename (source, dest) == 0)
  {
    return TRUE;
  }
  
  eclog_err_os (LL_ERROR, "ENTC", "move", "can't move '%s' to '%s'", source, dest);
  return FALSE;
}

//--------------------------------------------------------------------------------

int ecfs_cpfile (const EcString source, const EcString dest, EcErr err)
{
#ifdef __APPLE_CC__
  
  if (copyfile (source, dest, 0, COPYFILE_ACL | COPYFILE_XATTR | COPYFILE_DATA) != 0)
  {
    return ecerr_lastErrorOS (err, ENTC_LVL_ERROR);
  }
  
  return ENTC_ERR_NONE;
  
#elif __LINUX_OS
  
  int sfd = open (source, O_RDONLY, 0);
  int dfd = open (dest, O_WRONLY | O_CREAT /*| O_TRUNC/*/, 0644);
  
  // struct required, rationale: function stat() exists also
  struct stat stat_source;
  fstat(sfd, &stat_source);
  
  sendfile (dfd, sfd, 0, stat_source.st_size);
  
  close (sfd);
  close (dfd);
  
  return ENTC_ERR_NONE;

#else
  
  EcFileHandle fhin;
  EcFileHandle fhout;
  
  EcBuffer buf;
  
  fhin = ecfh_open (source, O_RDONLY);
  if (fhin == NULL)
  {
    return ecerr_lastErrorOS(err, ENTC_LVL_ERROR); 
  }
  
  fhout = ecfh_open (dest, O_WRONLY | O_CREAT);
  if (fhout == NULL)
  {
    ecfh_close (&fhin);
    
    return ecerr_lastErrorOS(err, ENTC_LVL_ERROR);     
  }
  
  buf = ecbuf_create (10000);
  
  // copy all data
  while (TRUE)
  {
    uint_t bytesRead = ecfh_readBuffer (fhin, buf);
    if (bytesRead > 0)
    {
      ecfh_writeBuffer (fhout, buf, bytesRead); 
    }
    else
    {
      break; 
    }
  }
  
  ecbuf_destroy (&buf);
  
  ecfh_close (&fhin);
  ecfh_close (&fhout);
  
  return ENTC_ERR_NONE;
#endif
}

//--------------------------------------------------------------------------------

int ecfs_mkdir (const EcString source)
{
  return mkdir (source, 0770) == 0;
}

//--------------------------------------------------------------------------------

#ifdef HAVE_FDOPENDIR

int ecfs_rmdir_dir (int fd);

//--------------------------------------------------------------------------------

int ecfs_rmdir_loop (int fd, DIR* dir)
{
  int res = TRUE;
  struct dirent* dentry;
  
  // iterate through all entries
  for (dentry = readdir (dir); res && isAssigned (dentry); dentry = readdir (dir))
  {
    eclogger_fmt (LL_TRACE, "ENTC", "rmdir", "rm dir %s", dentry->d_name);
    
    // ignore those 
    if (ecstr_equal (dentry->d_name, ".") || ecstr_equal (dentry->d_name, ".."))
    {
      continue;
    }
    
    {      
      switch (dentry->d_type)
      {
        // if this flag is set we need to check with stat (no info about type)
        case DT_UNKNOWN:
        {
          struct stat st;
          int fd2 = openat (fd, dentry->d_name, 0);
          if (fd2 < 0)
          {
            res = FALSE;
          }
          else
          {
            if (fstat (fd2, &st) == 0)
            {
              //res = unlinkat (fd, dentry->d_name, S_ISDIR (st.st_mode) ? AT_REMOVEDIR : 0) == 0;
            }
            else
            {
              res = FALSE;
            }
            
            close (fd2);
          }
        }
        break;
        // if directory
        case DT_DIR:
        {
          //res = unlinkat (fd, dentry->d_name, AT_REMOVEDIR) == 0;
        }
        break;
        default:
        {
          //res = unlinkat (fd, dentry->d_name, 0) == 0;
        }
        break;
      }
    }
  }
  return res;
}

//--------------------------------------------------------------------------------

int ecfs_rmdir_dir (int fd)
{
  int res;
  DIR* dir = fdopendir (fd);
  
  if (isNotAssigned (dir))
  {
    eclogger_err (LL_ERROR, "ENTC", "rmdir", errno, "can't open directory");
    return FALSE;
  }
  
  res = ecfs_rmdir_loop (fd, dir);
  
  closedir(dir); 
  
  return res;
}

//--------------------------------------------------------------------------------

int ecfs_rmdir (const EcString source, int forceOnNoneEmpty)
{
  int res = TRUE;
  // try to remove recusively
  if (forceOnNoneEmpty)
  {
    int fd = open (source, 0);
    if (fd < 0)
    {
      eclogger_err (LL_ERROR, "ENTC", "rmdir", errno, "can't open directory");
      return FALSE;
    }
    
    res = ecfs_rmdir_dir (fd);
    
    // clean up
    close (fd);
  }
  
  return res && (rmdir (source) == 0);
}

//--------------------------------------------------------------------------------

#else /* HAVE_FDOPENDIR */

//--------------------------------------------------------------------------------

int ecfs_rmdir_removeDir (const EcString source)
{
  //eclogger_fmt (LL_TRACE, "ENTC", "rmdir", "rm directory %s", source);
  
  if (rmdir (source) != 0)
  {
    eclog_err_os (LL_ERROR, "ENTC", "rmdir", "can't delete directory");
    return FALSE;
  }
  
  return TRUE;
}

//--------------------------------------------------------------------------------

int ecfs_rmdir_removeFile (const EcString source)
{
  //eclogger_fmt (LL_TRACE, "ENTC", "rmdir", "rm file %s", source);
  
  if (unlink (source) != 0)
  {
    eclog_err_os (LL_ERROR, "ENTC", "rmdir", "can't unlink file");
    return FALSE;
  }
  
  return TRUE;
}

//--------------------------------------------------------------------------------

int ecfs_rmdir_dir (const EcString source);

int ecfs_rmdir_loop (const EcString source, DIR* dir)
{
  int res = TRUE;
  struct dirent* dentry;
  
  // iterate through all entries
  for (dentry = readdir (dir); res && isAssigned (dentry); dentry = readdir (dir))
  {
    // ignore those
    if (ecstr_equal (dentry->d_name, ".") || ecstr_equal (dentry->d_name, ".."))
    {
      continue;
    }
    
    {
      EcString path = ecfs_mergeToPath (source, dentry->d_name);
      
      switch (dentry->d_type)
      {
          // if this flag is set we need to check with stat (no info about type)
        case DT_UNKNOWN:
        {
          ecfs_rmdir (path, TRUE);
        }
        break;
        // if directory
        case DT_DIR:
        {
          res = ecfs_rmdir (path, TRUE);
        }
        break;
        default:
        {
          res = ecfs_rmdir_removeFile (path);
        }
        break;
      }
      
      ecstr_delete (&path);      
    }
  }
  return res;
}

//--------------------------------------------------------------------------------

int ecfs_rmdir_dir (const EcString source)
{
  int res;
  DIR* dir = opendir (source);
  
  if (isNotAssigned (dir))
  {
    eclog_err_os (LL_ERROR, "ENTC", "rmdir", "can't open directory");
    return FALSE;
  }
  
  res = ecfs_rmdir_loop (source, dir);
  
  closedir(dir);  
  
  return res;
}

//--------------------------------------------------------------------------------

int ecfs_rmdir (const EcString source, int forceOnNoneEmpty)
{
  struct stat st;
  
  if (stat (source, &st) == 0)
  {
    if (S_ISDIR (st.st_mode))
    {
      if (forceOnNoneEmpty)
      {
        ecfs_rmdir_dir (source);
      }
      
      return ecfs_rmdir_removeDir (source);
    }
    else
    {
      return ecfs_rmdir_removeFile (source);
    }
  }
  else
  {
    
    return FALSE;
  }
}

//--------------------------------------------------------------------------------

/*
int ecfs_rmdir (const EcString source, int forceOnNoneEmpty)
{
  // try to remove recusively
  if (forceOnNoneEmpty)
  {
    return ecfs_rmdir_dir (source);
  }
  else
  {
    return (rmdir (source) == 0);
  }
}
 */

//--------------------------------------------------------------------------------

#endif /* HAVE_FDOPENDIR */

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
  
  info->size = st.st_size;
  info->inode = st.st_ino;

  info->cdate = st.st_ctime;
  info->mdate = st.st_mtime;
  info->adate = st.st_atime;
  
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
    uint_t diff02;
    // find the difference between the found and original
    uint_t diff01 = found - file;
    // allocate some memory
    resdir = (char*)malloc( sizeof(char) * (diff01 + 1) );
    // copy the dir part
    strncpy(resdir, file, diff01);
    // increase found
    found++;
    
    diff02 = strlen(found) + 1;
    // allocate the memory for file part
    resfile = (char*)malloc(diff02);
    // copy file part
    strncpy(resfile, found, diff02);
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
#ifdef __APPLE__
  char pathbuf[PATH_MAX + 1];
  unsigned int bufsize = sizeof(pathbuf);
  
  _NSGetExecutablePath (pathbuf, &bufsize);
  
  return ecfs_getDirectory (pathbuf);
#else
  const EcString execPath = argv[0];
  return ecfs_getDirectory (execPath);  
#endif
}

//------------------------------------------------------------------------------------------------------------

void ecfs_getExecutable (EcString* ppath, EcString* pname, int argc, char *argv[])
{
#ifdef __APPLE__
  // reserve executable path
  char buffer [PATH_MAX + 1];
  unsigned int bufsize = sizeof(buffer);
  
  _NSGetExecutablePath (buffer, &bufsize);
  
  // override values
  ecstr_replaceTO (ppath, ecfs_getDirectory (buffer));
  ecstr_replaceTO (pname, ecfs_extractFileName (buffer));
#else
  // override values
  ecstr_replaceTO (ppath, ecfs_getDirectory (argv[0]));
  ecstr_replaceTO (pname, ecfs_extractFileName (argv[0]));
#endif
}

//-----------------------------------------------------------------------------------

int ecfs_createDirIfNotExists (const EcString path)
{
  int res = mkdir (path, 0770);    
  return res == 0 || errno == EEXIST;
}

//------------------------------------------------------------------------------------------------------------

int ecfs_chdir (const EcString path, EcErr err)
{
  if (chdir (path) != 0)
  {
    return ecerr_lastErrorOS (err, ENTC_LVL_ERROR);
  }
  else
  {
    return ENTC_ERR_NONE;
  }
}

//-------------------------------------------------------------------------

int ecfs_exists (const EcString path)
{
  struct stat st;
  
  if( !path )
  {
    return FALSE;
  }
  
  return stat (path, &st) == 0;
}

//-------------------------------------------------------------------------

#endif
