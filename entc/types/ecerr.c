#include "ecerr.h"

#include <stdio.h>

// entc includes
#include "types/ecstring.h"

//-----------------------------------------------------------------------------

EcErr ecerr_create ()
{
  EcErr self = ENTC_NEW(struct EcErr_s);
  
  self->code = 0;
  self->level = 0;
  self->text = NULL;
  
  return self;
}

//-----------------------------------------------------------------------------

void ecerr_destroy (EcErr* pself)
{
  EcErr self = *pself;
  
  ecstr_delete (&(self->text));
  
  ENTC_DEL(pself, struct EcErr_s);
}

//-----------------------------------------------------------------------------

int ecerr_set (EcErr self, int lvl, int code, const char* text)
{
  if (self == NULL)
  {
    return code;
  }
  
  if (self->text)
  {
    // catenate
    ecstr_replaceTO (&(self->text), ecstr_catc(self->text, '|', text));
  }
  else
  {
    self->text = ecstr_copy (text);
  }
  
  self->code = code;
  
  return code;
}

//-----------------------------------------------------------------------------

int ecerr_setTO (EcErr self, int lvl, int code, char** ptext)
{
  if (self == NULL)
  {
    return code;
  }
  
  if (self->text)
  {
    // catenate
    ecstr_replaceTO (&(self->text), ecstr_catc(self->text, '|', *ptext));
    
    ecstr_delete(ptext);
  }
  else
  {
    self->text = *ptext;
    *ptext = NULL;
  }
  
  return code;
}

//-----------------------------------------------------------------------------

#if defined _WIN64 || defined _WIN32

#include <windows.h>

//-----------------------------------------------------------------------------

int ecerr_formatErrorOS (EcErr self, int lvl, unsigned long errCode)
{
  if (errCode)
  {
    LPTSTR buffer = NULL;
    DWORD res = FormatMessageA (FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, errCode, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&buffer, 0, NULL);
    
    if (buffer)
    {
      if (res > 0)
      {
        ecerr_set (self, lvl, Q6ERR_OS_ERROR, buffer);
      }
      // release buffer
      LocalFree (buffer);
    }
  }
  
  return Q6ERR_OS_ERROR;
}

//-----------------------------------------------------------------------------

int ecerr_lastErrorOS (EcErr self, int lvl)
{
  if (self == NULL)
  {
    return ENTC_ERR_OS_ERROR;
  }
  
  return ecerr_formatErrorOS (self, lvl, GetLastError ());
}

//-----------------------------------------------------------------------------

#else

#include <errno.h>
#include <string.h>

//-----------------------------------------------------------------------------

int ecerr_formatErrorOS (EcErr self, int lvl, unsigned long errCode)
{
  return ecerr_set (self, lvl, ENTC_ERR_OS_ERROR, strerror(errCode));
}

//-----------------------------------------------------------------------------

int ecerr_lastErrorOS (EcErr self, int lvl)
{
  if (self == NULL)
  {
    return ENTC_ERR_NONE;
  }
  
  return ecerr_formatErrorOS (self, lvl, errno);
}

//-----------------------------------------------------------------------------

#endif


