/*
 Copyright (c) 2019 Alexander Kalkhof
 
 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:
 
 The above copyright notice and this permission notice shall be included in all
 copies or substantial portions of the Software.
 
 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 SOFTWARE.
 */

#include "entc_err.h"

// entc includes
#include "stc/entc_str.h"

// c includes
#include <string.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>

#if defined _WIN64 || defined _WIN32
#include <windows.h>
#else
#include <errno.h>
#endif

//-----------------------------------------------------------------------------

struct EntcErr_s
{
    char* text;
    
    unsigned long code;
    
};

//-----------------------------------------------------------------------------

EntcErr entc_err_new (void)
{
  EntcErr self = malloc (sizeof(struct EntcErr_s));

  self->text = NULL;
  self->code = ENTC_ERR_NONE;
  
  return self;    
}

//-----------------------------------------------------------------------------

void entc_err_del (EntcErr* p_self)
{
  EntcErr self = *p_self;
  *p_self = NULL;
  
  entc_str_del (&(self->text));
  
  free (self);
}

//-----------------------------------------------------------------------------

const char* entc_err_text (EntcErr self)
{
  return self->text;    
}

//-----------------------------------------------------------------------------

unsigned long entc_err_code (EntcErr self)
{
  return self->code;
}

//-----------------------------------------------------------------------------

int entc_err_set (EntcErr self, unsigned long code, const char* error_message)
{
  self->code = code;
  self->text = entc_str_cp (error_message);
  
  return code;
}

//-----------------------------------------------------------------------------

int entc_err_set_fmt (EntcErr self, unsigned long code, const char* error_message, ...)
{
  char buffer [1002];
  
  // variables
  va_list ptr;
  
  if (self == NULL)
  {
    return code;
  }
  
  va_start(ptr, error_message);
  
#if defined _WIN64 || defined _WIN32
  vsnprintf_s (buffer, 1001, 1000, error_message, ptr);
#else
  vsnprintf (buffer, 1000, error_message, ptr);
#endif
  
  entc_err_set (self, code, buffer);
  
  va_end(ptr);
  
  return code;
}

//-----------------------------------------------------------------------------

int entc_err_formatErrorOS (EntcErr self, unsigned long errCode)
{
#if defined _WIN64 || defined _WIN32
  LPTSTR buffer = NULL;
  DWORD res = FormatMessageA (FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, errCode, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&buffer, 0, NULL);
  
  if (buffer)
  {
    if (res > 0)
    {
      entc_err_set (self, ENTC_ERR_OS, buffer);
    }
    // release buffer
    LocalFree (buffer);
  }

  return ENTC_ERR_OS;
#else
  return entc_err_set (self, ENTC_ERR_OS, strerror(errCode));
#endif
}

//-----------------------------------------------------------------------------

int entc_err_lastOSError (EntcErr self)
{
#if defined _WIN64 || defined _WIN32
  return entc_err_formatErrorOS (self, GetLastError ());
#else
  return entc_err_formatErrorOS (self, errno);
#endif
}

//-----------------------------------------------------------------------------

