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

#include "../system/macros.h"
#include "../system/types.h"

#include "../types/ecstring.h"
#include "../types/ecbuffer.h"

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

typedef struct
{
  
  EcString name;
  
  ubyte_t type;
  
  uint64_t size;
    
  // creat date
  time_t cdate;
  
  // last modification date
  time_t mdate;
  
  // access date
  time_t adate;
  
} EcFileInfo_s; typedef EcFileInfo_s* EcFileInfo;


__CPP_EXTERN______________________________________________________________________________START

__LIB_EXPORT int ecfs_move (const EcString source, const EcString dest);

__LIB_EXPORT int ecfs_copy (const EcString source, const EcString dest);

__LIB_EXPORT int ecfs_mkdir (const EcString source);

__LIB_EXPORT int ecfs_rmdir (const EcString source, int forceOnNoneEmpty);

__LIB_EXPORT int ecfs_rmfile (const EcString source);

__LIB_EXPORT int ecfs_fileInfo (EcFileInfo, const EcString path);

__LIB_EXPORT int ecfs_createDirIfNotExists (const EcString path);

__LIB_EXPORT int ecfs_exists (const EcString path);

/* **** path string operations **** */

__LIB_EXPORT EcString ecfs_getExecutablePath (int argc, char *argv[]);

__LIB_EXPORT EcString ecfs_getRealPath(const EcString path);

__LIB_EXPORT EcString ecfs_mergeToPath(const EcString path, const EcString file);

__LIB_EXPORT const EcString ecfs_extractFile(const EcString path);

__LIB_EXPORT EcString ecfs_getCurrentDirectory();

__LIB_EXPORT EcString ecfs_getDirectory(const EcString filename);

__LIB_EXPORT void ecfs_basedir(const EcString basedir, const EcString file, EcString* ptr_resdir, EcString* ptr_resfile);

__LIB_EXPORT const EcString ecfs_extractFileExtension(const EcString path);

__LIB_EXPORT EcString ecfs_extractFileName(const EcString path);

__CPP_EXTERN______________________________________________________________________________END

struct EcFileHandle_s; typedef struct EcFileHandle_s* EcFileHandle;

__CPP_EXTERN______________________________________________________________________________START

__LIB_EXPORT EcFileHandle ecfh_open(const EcString filename, int flags);
  
__LIB_EXPORT void ecfh_close(EcFileHandle*);
  
__LIB_EXPORT int ecfh_writeString (EcFileHandle, const EcString);
  
__LIB_EXPORT int ecfh_writeBuffer (EcFileHandle, const EcBuffer buffer, uint_t size);

__LIB_EXPORT int ecfh_writeConst (EcFileHandle, const char* buffer, uint_t size);

__LIB_EXPORT uint_t ecfh_readBuffer(EcFileHandle, EcBuffer buffer);
  
__LIB_EXPORT void ecfh_reset(EcFileHandle);

__LIB_EXPORT int ecfh_fileno(EcFileHandle);

__LIB_EXPORT EcString ecfh_md5(EcFileHandle);
  
__LIB_EXPORT FILE* ecfh_file(EcFileHandle, EcString type);

__CPP_EXTERN______________________________________________________________________________END

typedef struct EcDirHandle_s* EcDirHandle;  

__CPP_EXTERN______________________________________________________________________________START

__LIB_EXPORT EcDirHandle ecdh_create (const EcString path);
  
__LIB_EXPORT void ecdh_destroy (EcDirHandle*);
  
__LIB_EXPORT int ecdh_next (EcDirHandle, EcFileInfo*, int fullInfo);
  
// fills a list with all files found -> filename as list entry
__LIB_EXPORT int ecdh_scan (const EcString path, EcList entries, int filetype);
  
__LIB_EXPORT void ecdh_seekType (const EcString path, EcFileInfo entry);

__LIB_EXPORT void ecfi_clone (const EcFileInfo, EcFileInfo);

__CPP_EXTERN______________________________________________________________________________END
  
#endif
