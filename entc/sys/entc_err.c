#include "cape_err.h"

// cape includes
#include "stc/cape_str.h"

// c includes
#include <malloc.h>
#include <errno.h>
#include <string.h>
#include <stdarg.h>

//-----------------------------------------------------------------------------

struct CapeErr_s
{
    char* text;
    
    unsigned long code;
    
};

//-----------------------------------------------------------------------------

CapeErr cape_err_new (void)
{
  CapeErr self = malloc (sizeof(struct CapeErr_s));

  self->text = NULL;
  self->code = CAPE_ERR_NONE;
  
  return self;    
}

//-----------------------------------------------------------------------------

void cape_err_del (CapeErr* p_self)
{
  CapeErr self = *p_self;
  *p_self = NULL;
  
  cape_str_del (&(self->text));
  
  free (self);
}

//-----------------------------------------------------------------------------

const char* cape_err_text (CapeErr self)
{
  return self->text;    
}

//-----------------------------------------------------------------------------

unsigned long cape_err_code (CapeErr self)
{
  return self->code;
}

//-----------------------------------------------------------------------------

int cape_err_set (CapeErr self, unsigned long code, const char* error_message)
{
  self->code = code;
  self->text = cape_str_cp (error_message);
  
  return code;
}

//-----------------------------------------------------------------------------

int cape_err_set_fmt (CapeErr self, unsigned long code, const char* error_message, ...)
{
  char buffer [1002];
  
  // variables
  va_list ptr;
  
  if (self == NULL)
  {
    return code;
  }
  
  va_start(ptr, error_message);
  
#ifdef _WIN32
  vsnprintf_s (buffer, 1001, 1000, text, ptr);
#else
  vsnprintf (buffer, 1000, error_message, ptr);
#endif
  
  cape_err_set (self, code, buffer);
  
  va_end(ptr);
  
  return code;
}

//-----------------------------------------------------------------------------

int cape_err_formatErrorOS (CapeErr self, unsigned long errCode)
{
  return cape_err_set (self, CAPE_ERR_OS, strerror(errCode));
}

//-----------------------------------------------------------------------------

int cape_err_lastOSError (CapeErr self)
{
  return cape_err_formatErrorOS (self, errno);
}

//-----------------------------------------------------------------------------

