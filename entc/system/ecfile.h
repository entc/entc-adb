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

#include <sys/stat.h>
#include <stdio.h>
#include <fcntl.h>

#include "../system/macros.h"

/* definitions for all platforms */
#define ENTC_FILETYPE_ISNONE  0
#define ENTC_FILETYPE_ISDIR   1
#define ENTC_FILETYPE_ISFILE  2


#include "../types/ecstring.h"

#ifdef _WIN32

#include <stdio.h>
#include <io.h>
#include <direct.h>

#define ENTC_PATH_SEPARATOR '\\'

#define EcStatInfo struct __stat64

/* define the folder functions */

struct EcDirHandle_s; typedef struct EcDirHandle_s* EcDirHandle;

struct EcDirNode_s
{

  uint_t d_type;

  EcString d_name;

};

typedef struct EcDirNode_s* EcDirNode;
  
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

struct EcDirNode_s
{

  uint_t d_type;

  EcString d_name;

};

typedef struct EcDirNode_s* EcDirNode;
  
#define DT_DIR FILE_ATTRIBUTE_DIRECTORY

#define mode_t int

#else

#include <dirent.h>
#include <unistd.h>

#define ENTC_PATH_SEPARATOR '/'

typedef int EcOsHandle;

typedef struct stat EcStatInfo;

/* define the folder functions */


typedef DIR*              EcDirHandle;
typedef struct dirent*    EcDirNode;

#endif /* else _WIN32 */

struct EcFileHandle_s;

typedef struct EcFileHandle_s* EcFileHandle;

#ifndef S_ISREG
#define S_ISREG(m) ((m) & _S_IFREG)
#endif

#ifndef S_ISFIFO
#define S_ISFIFO(m) ((m) & _S_IFIFO)
#endif

/* start with some methods */


__CPP_EXTERN______________________________________________________________________________START

  /* **** file-handle methods **** */
  
__LIB_EXPORT EcFileHandle ecfh_open(const EcString filename, int flags);
  
__LIB_EXPORT void ecfh_close(EcFileHandle*);
  
__LIB_EXPORT void ecfh_writeString(EcFileHandle, const EcString);
  
__LIB_EXPORT void ecfh_writeBuffer(EcFileHandle, const EcBuffer buffer, uint_t size);

__LIB_EXPORT void ecfh_writeConst(EcFileHandle, const char* buffer, uint_t size);

__LIB_EXPORT uint_t ecfh_readBuffer(EcFileHandle, EcBuffer buffer);
  
__LIB_EXPORT void ecfh_reset(EcFileHandle);

__LIB_EXPORT int ecfh_fileno(EcFileHandle);

__LIB_EXPORT EcString ecfh_md5(EcFileHandle);
  
__LIB_EXPORT FILE* ecfh_file(EcFileHandle, EcString type);

  /* **** dir-handle methods **** */
  
__LIB_EXPORT EcDirHandle ecdh_new(const EcString path);
  
__LIB_EXPORT void ecdh_close(EcDirHandle*);
  
__LIB_EXPORT int ecdh_next(EcDirHandle, EcDirNode*);
  
__LIB_EXPORT int ecdh_getFileType(const EcString path, EcDirNode);

// fills a list with all files found -> filename as list entry
__LIB_EXPORT int ecdh_scan (const EcString path, EcList entries, int filetype);
  
  /* **** filesystem operations **** */
  
__LIB_EXPORT int ecfs_move(const EcString source, const EcString dest);
  
__LIB_EXPORT int ecfs_mkdir(const EcString source);

__LIB_EXPORT int ecfs_rmdir(const EcString source);

__LIB_EXPORT int ecfs_rmfile(const EcString source);

__LIB_EXPORT int ecfs_stat(EcStatInfo*, const EcString path);

  /* **** path string operations **** */
  
__LIB_EXPORT EcString ecfs_getRealPath(const EcString path);

__LIB_EXPORT EcString ecfs_mergeToPath(const EcString path, const EcString file);

__LIB_EXPORT const EcString ecfs_extractFile(const EcString path);
  
__LIB_EXPORT EcString ecfs_getCurrentDirectory();

__LIB_EXPORT EcString ecfs_getExecutablePath (void);
  
__LIB_EXPORT EcString ecfs_getDirectory(const EcString filename);
  
__LIB_EXPORT void ecfs_basedir(const EcString basedir, const EcString file, EcString* ptr_resdir, EcString* ptr_resfile);

__LIB_EXPORT const EcString ecfs_extractFileExtension(const EcString path);
  
__LIB_EXPORT EcString ecfs_extractFileName(const EcString path);
  
__CPP_EXTERN______________________________________________________________________________END

#endif
