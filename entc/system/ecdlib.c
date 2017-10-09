#include "ecdlib.h"

#include "macros.h"

#if defined _WIN64 || defined _WIN32

#include <windows.h>

//-----------------------------------------------------------------------------

struct EcDl_s
{
  char* name;
  
  char* path;
  
  HMODULE ptr;
  
};

#else

#include <dlfcn.h>

//-----------------------------------------------------------------------------

struct EcDl_s
{
  char* name;
  
  char* path;
  
  void* ptr;
  
};

#endif

#include <stdio.h>


//-----------------------------------------------------------------------------

EcDl ecdl_create (const EcString name, const EcString path)
{
  EcDl self = ENTC_NEW(struct EcDl_s);
  
  self->name = ecstr_copy(name);
  self->path = ecstr_copy(path);
  
  self->ptr = NULL;
  
  return self;
}

//-----------------------------------------------------------------------------

void ecdl_destroy (EcDl* pself)
{
  EcDl self = *pself;
  
  free (self);
}

//-----------------------------------------------------------------------------

int ecdl_load (EcDl self, EcErr err)
{
  // construct dll name
  char buffer [1024];
  
  if (self->path)
  {
    snprintf (buffer, 1024, "%s/%s.dll", self->path, self->name);
  }
  else
  {
    snprintf (buffer, 1024, "%s.dll", self->name);
  }
  
#if defined _WIN64 || defined _WIN32
  
  self->ptr = LoadLibrary (buffer);
  
#else
  
  self->ptr = dlopen (buffer, RTLD_NOW);

#endif

  if (self->ptr == NULL)
  {
    return ecerr_lastErrorOS (err, ENTC_LVL_ERROR);
  }
  
  return ENTC_ERR_NONE;
}

//-----------------------------------------------------------------------------

int ecdl_unload (EcDl self, EcErr err)
{
  if (self->ptr == NULL)
  {
    return ecerr_set (err, ENTC_LVL_ERROR, ENTC_ERR_WRONG_STATE, "library was not loaded");
  }
  
#if defined _WIN64 || defined _WIN32

  if(FreeLibrary (self->ptr) == 0)
  {
    // failed
    return syserr_lastErrorOS (err, __ERRL_ERROR);
  }

#else
  
  dlclose (self->ptr);
  
#endif
  
  self->ptr = NULL;
  return ENTC_ERR_NONE;
}

//-----------------------------------------------------------------------------

typedef void (*fct_dummy) (void);

int ecdl_assign (EcDl self, EcErr err, void* buffer, int n, ...)
{
  int i;
  char* method;
  fct_dummy* fd = buffer;
  
  // variables
  va_list ptr;
  
  va_start(ptr, n);
  
  for (i = 0; i < n; i++)
  {
    fct_dummy mptr;
    
    method = va_arg (ptr, char*);
    
    //printf("check for method: '%s'\n", method);
    
#if defined _WIN64 || defined _WIN32

    mptr = GetProcAddress(self->ptr, method);
    if (mptr == NULL)
    {
      return ecerr_set (err, ENTC_LVL_ERROR, ENTC_ERR_WRONG_STATE, "can't find method in module");
    }

#else
    
    {
      int res;
      char* errCode;
      // clear error code
      dlerror();
      
      res = dlsym (self->ptr, method);
      
      errCode = dlerror();
      // check if an error ocours
      if (isAssigned(errCode))
      {
        return ecerr_set (err, ENTC_LVL_ERROR, ENTC_ERR_WRONG_STATE, "can't find method in module");
      }
    }
    
#endif
    
    *fd = mptr;
    fd++;
  }
  
  va_end(ptr);
  
  return ENTC_ERR_NONE;
}

//-----------------------------------------------------------------------------
