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

#ifndef ENTC_UTILS_SECURE_FILE_H
#define ENTC_UTILS_SECURE_FILE_H 1

#include "../system/macros.h"
#include "../system/types.h"

#include "../system/ecfile.h"
#include "../types/ecstring.h"

/* opens a file and returns the success
 this is the secure access, try to use this version only
 
 secure breaches are reported to the logger
 
 access to all files withing the confdir is ok
 
 access outside the confdir will be reported
 access to critical files/folders will be reported as security breach
 
 the struct EcSecFopen will contain the real filename and the file descriptor
 */
struct EcSecFopen
{
  EcFileHandle fhandle;
  
  EcString filename;
  
  int os_error;
  
  int sec_error;
  
  time_t mtime;
#ifdef _WIN32
  __int64 size;
#else
  off_t size;
#endif
};

/* opens a dir and returns the success
 for more infos see qsec_fopen
 */
struct EcSecDopen
{
  EcDirHandle dh;
  
  EcString path;
  
  int sec_error;
  
  time_t mtime;
  
};


__CPP_EXTERN______________________________________________________________________________START
    
__LIB_EXPORT int ecsec_fopen(struct EcSecFopen*, const EcString filename, int flags, const EcString confdir);
  
__LIB_EXPORT int ecsec_dopen(struct EcSecDopen*, const EcString path, const EcString confdir);

  /* utility methods */
__LIB_EXPORT void ecsec_mkdir(const EcString path, const EcString confdir);
  
__CPP_EXTERN______________________________________________________________________________END

#endif
