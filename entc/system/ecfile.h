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

#ifndef ENTC_SYSTEM_FILE_H
#define ENTC_SYSTEM_FILE_H 1

//-----------------------------------------------------------------------------

#include "sys/entc_export.h"
#include "sys/entc_types.h"

#include "types/ecstring.h"
#include "types/ecbuffer.h"
#include "types/eclist.h"

#include "types/ecerr.h"

//-----------------------------------------------------------------------------

/* definitions for all platforms */
#define ENTC_FILETYPE_ISNONE  0
#define ENTC_FILETYPE_ISDIR   1
#define ENTC_FILETYPE_ISFILE  2

#ifdef _WIN32

#include <stdio.h>
#include <io.h>
#include <direct.h>
#include <fcntl.h>

#define ENTC_PATH_SEPARATOR '\\'

#define EcStatInfo struct __stat64

/* define the folder functions */

struct EcDirHandle_s; typedef struct EcDirHandle_s* EcDirHandle;

  
#define DT_DIR FILE_ATTRIBUTE_DIRECTORY

#define mode_t int

#elif defined ENTC_PLATFORM_DOS

#include <stdio.h>
#include <io.h>
#include <dos.h>
#include <direct.h>

typedef int EcOsHandle;

#define ENTC_PATH_SEPARATOR 92

typedef struct find_t QCStatInfo;

/* define the folder functions */

struct QCDirHandle_s;

typedef struct EcDirHandle_s* EcDirHandle;

  
#define DT_DIR FILE_ATTRIBUTE_DIRECTORY

#define mode_t int

#else

#include <stdio.h>
#include <dirent.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/time.h>

#define ENTC_PATH_SEPARATOR '/'

typedef int EcOsHandle;


/* define the folder functions */


#endif /* else _WIN32 */


#ifndef S_ISREG
#define S_ISREG(m) ((m) & _S_IFREG)
#endif

#ifndef S_ISFIFO
#define S_ISFIFO(m) ((m) & _S_IFIFO)
#endif

/* start with some methods */

//-----------------------------------------------------------------------------

#pragma pack(push, 16)
typedef struct
{
  
  EcString name;
  
  EcString path;
 
  ubyte_t type;
  
  uint64_t size;
    
  // creat date
  time_t cdate;
  
  // last modification date
  time_t mdate;
  
  // access date
  time_t adate;
  
  uint64_t inode;
  
} EcFileInfo_s; typedef EcFileInfo_s* EcFileInfo;
#pragma pack(pop)

//=============================================================================

__ENTC_LIBEX int ecfs_move (const EcString source, const EcString dest);

__ENTC_LIBEX int ecfs_cpfile (const EcString source, const EcString dest, EcErr err);

__ENTC_LIBEX int ecfs_cpdir (const EcString source, const EcString dest, EcErr err);

__ENTC_LIBEX int ecfs_mkdir (const EcString source);

__ENTC_LIBEX int ecfs_rmdir (const EcString source, int forceOnNoneEmpty);

__ENTC_LIBEX int ecfs_rmfile (const EcString source);

__ENTC_LIBEX int ecfs_fileInfo (EcFileInfo, const EcString path);

__ENTC_LIBEX int ecfs_createDirIfNotExists (const EcString path);

__ENTC_LIBEX int ecfs_exists (const EcString path);

__ENTC_LIBEX int ecfs_chdir (const EcString path, EcErr err);

/* **** path string operations **** */

__ENTC_LIBEX EcString ecfs_getExecutablePath (int argc, char *argv[]);

__ENTC_LIBEX void ecfs_getExecutable (EcString* path, EcString* name, int argc, char *argv[]);

__ENTC_LIBEX EcString ecfs_getRealPath(const EcString path);   // finds the absolute path (path needs to exists)

__ENTC_LIBEX EcString ecfs_mergeToPath(const EcString path, const EcString file);

__ENTC_LIBEX const EcString ecfs_extractFile(const EcString path);

__ENTC_LIBEX EcString ecfs_getCurrentDirectory();

__ENTC_LIBEX EcString ecfs_getDirectory(const EcString filename);

__ENTC_LIBEX void ecfs_basedir(const EcString basedir, const EcString file, EcString* ptr_resdir, EcString* ptr_resfile);

__ENTC_LIBEX const EcString ecfs_extractFileExtension(const EcString path);

__ENTC_LIBEX EcString ecfs_extractFileName(const EcString path);

//-----------------------------------------------------------------------------

__ENTC_LIBEX EcString         entc_fs_path_absolute           (const EcString relative_or_absolute_path);      // cleans the path and converts the path to an absolute path

//=============================================================================

struct EcFileHandle_s; typedef struct EcFileHandle_s* EcFileHandle;

//-----------------------------------------------------------------------------

__ENTC_LIBEX EcFileHandle ecfh_open (const EcString filename, int flags);
  
__ENTC_LIBEX void ecfh_close (EcFileHandle*);

__ENTC_LIBEX uint64_t ecfh_size (EcFileHandle);
  
__ENTC_LIBEX int ecfh_writeString (EcFileHandle, const EcString);
  
__ENTC_LIBEX int ecfh_writeString_encrypted (EcFileHandle, const EcString, const EcString key);

__ENTC_LIBEX int ecfh_writeBuffer (EcFileHandle, const EcBuffer buffer, uint_t size);

__ENTC_LIBEX int ecfh_writeConst (EcFileHandle, const char* buffer, uint_t size);

__ENTC_LIBEX uint_t ecfh_readBuffer (EcFileHandle, EcBuffer buffer);

__ENTC_LIBEX uint_t ecfh_readBufferOf (EcFileHandle, EcBuffer buffer, uint_t offset);

__ENTC_LIBEX void ecfh_reset(EcFileHandle);

__ENTC_LIBEX int ecfh_fileno(EcFileHandle);

__ENTC_LIBEX EcString ecfh_md5(EcFileHandle);
  
__ENTC_LIBEX FILE* ecfh_file (EcFileHandle, EcString type);

//=============================================================================

typedef struct EcDirHandle_s* EcDirHandle;  

//-----------------------------------------------------------------------------

__ENTC_LIBEX EcDirHandle ecdh_create (const EcString path);
  
__ENTC_LIBEX void ecdh_destroy (EcDirHandle*);
  
__ENTC_LIBEX int ecdh_next (EcDirHandle, EcFileInfo*, int fullInfo);

__ENTC_LIBEX void ecdh_seekType (const EcString path, EcFileInfo entry);

__ENTC_LIBEX void ecfi_clone (const EcFileInfo, EcFileInfo);

__ENTC_LIBEX const EcString ecdh_path (EcDirHandle);

__ENTC_LIBEX uint64_t ecdh_size (EcDirHandle, int recursive);

//-----------------------------------------------------------------------------

// fills a list with all files found -> filename as list entry
__ENTC_LIBEX int ecdh_scan (const EcString path, EcList entries, int filetype);

//-----------------------------------------------------------------------------
 
#endif
