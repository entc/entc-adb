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

#include "ecdl.h"

#include "system/ecdefs.h"
#include "system/ecfile.h"
#include "tools/eclog.h"

#ifdef _WIN32
#include <windows.h>

struct EcLibraryHandle_s
{
  
  HMODULE handle;
  
};

#else
#include <dlfcn.h>

struct EcLibraryHandle_s
{
  
  void* handle;
    
};

#endif /* _WIN32 */


/*------------------------------------------------------------------------*/

EcLibraryHandle ecdl_new (const EcString filename)
{
  /* variables */
  EcLibraryHandle self;
  const EcString fext;
  
  if( !ecstr_valid(filename) )
  {
    return 0;
  }
  
  fext = ecfs_extractFileExtension(filename);
  
  if( !ecstr_valid(fext) )
  {
    return 0;
  }
  
  /* check the file postfix */
#ifdef __APPLE_CC__
  if( !ecstr_equal(fext, "dylib") )
  {
    return 0;  
  }
#elif defined __LINUX_OS || defined __BSD_OS
  if( !ecstr_equal(fext, "so") )
  {
    return 0;  
  }  
#elif _WIN32
  if( !ecstr_equal(fext, "dll") )
  {
    return 0;  
  }  
#else
  return 0;
#endif
  {
#ifdef _WIN32
    HMODULE handle = LoadLibrary(filename);
#else
    // clear any existing errors
//    dlerror();
    
    void* handle = dlopen(filename, RTLD_NOW);
#endif
  
    if (isAssigned(handle))
    {
      self = ENTC_NEW(struct EcLibraryHandle_s);
      
      self->handle = handle;
      
      return self;
    }
    else
    {
#ifdef _WIN32
      eclog_err_os (LL_WARN, "_SYS", "dl", "can't load library");
#else
      eclog_fmt (LL_WARN, "_SYS", "dl", "can't load library: %s", dlerror());
#endif
	}
    return 0;
  }
}

/*------------------------------------------------------------------------*/

EcLibraryHandle ecdl_fromName (const EcString path, const EcString name)
{
  EcLibraryHandle ret = 0;
#ifdef __APPLE_CC__
  
  EcString fullname = ecstr_cat3("lib", name, ".dylib");

#elif defined __LINUX_OS || defined __BSD_OS
  
  EcString fullname = ecstr_cat3("lib", name, ".so");

#elif _WIN32
  
  EcString fullname = ecstr_cat2(name, ".dll");  

#else
  
  return 0;

#endif
  EcString fullpath = ecfs_mergeToPath(path, fullname);
  
#ifdef _WIN32
  HMODULE handle = LoadLibrary(fullpath);
#else
  void* handle = dlopen(fullpath, RTLD_NOW | RTLD_GLOBAL);
#endif
  
  if( handle )
	{
    EcLibraryHandle self = ENTC_NEW(struct EcLibraryHandle_s);
    
    self->handle = handle;

    ecstr_delete(&fullname);
    ecstr_delete(&fullpath);
    
    ret = self;
	}
  else
  {
#ifdef _WIN32
    eclog_err_os (LL_WARN, "_SYS", "dl", "can't load library '%s' from '%s'", name, fullpath);
#else
    eclog_fmt (LL_WARN, "_SYS", "dl", "can't load library '%s' : %s", name, dlerror());
#endif
  }
  
  ecstr_delete(&fullname);
  ecstr_delete(&fullpath);
  
  return ret;
}

/*------------------------------------------------------------------------*/

void ecdl_delete(EcLibraryHandle* pself)
{
  EcLibraryHandle self = *pself;
  
  if(self == NULL)
  {
    return;
  }

#ifdef _WIN32
  FreeLibrary(self->handle);
#else
  dlclose(self->handle);
#endif
  
  ENTC_DEL(pself, struct EcLibraryHandle_s);
}

/*------------------------------------------------------------------------*/

void* ecdl_method(EcLibraryHandle self, const EcString name)
{
  void* ret;
  
#ifdef _WIN32
  ret = GetProcAddress(self->handle, name);
#else
  char* err;
  // clear error code  
  dlerror();
  
  ret = dlsym(self->handle, name);
  
  err = dlerror();
  // check if an error ocours
  if (isAssigned(err)) {
    ret = 0;
  }
#endif
  
  return ret;
}

/*------------------------------------------------------------------------*/
