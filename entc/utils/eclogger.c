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

#include "eclogger.h"

#include "system/ectime.h"
#include "ecmessages.h"

#ifdef __DOS__

#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <mem.h>
#include <io.h>
#include <dos.h>
#include <ctype.h>
#include <conio.h>
#include <time.h>

#else

#include "../system/ecmutex.h"
#include "../system/ecfile.h"
#include "ecsecfile.h"

#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <errno.h>
#include <stdarg.h>
#include <string.h>
#include <fcntl.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <sys/time.h>
#endif
#endif

//----------------------------------------------------------------------------------------

void eclogger_msg (EcLogLevel lvl, const char* unit, const char* method, const char* msg)
{
  EcMessageData data;
  
  data.type = ENTC_MSGTYPE_LOG; data.rev = 1;
  data.ref = lvl;
  
  data.content = ecudc_create (ENTC_UDC_NODE, NULL);
  
  // better do the text format in the listeners
  ecudc_add_asString (data.content, "unit", unit);
  ecudc_add_asString (data.content, "method", method);
  ecudc_add_asString (data.content, "msg", msg);
  
  // send the message by messaging system
  ecmessages_broadcast (ENTC_MSGSRVC_LOG, &data, NULL);
  
  ecudc_destroy (&(data.content));
}

//----------------------------------------------------------------------------------------

void eclogger_fmt (EcLogLevel lvl, const char* unit, const char* method, const char* format, ...)
{
  char buffer [1002];
  // variables
  va_list ptr;

  va_start(ptr, format);

#ifdef _WIN32
  vsnprintf_s (buffer, 1001, 1000, format, ptr);
#else
  vsnprintf (buffer, 1000, format, ptr);
#endif 

  eclogger_msg (lvl, unit, method, buffer);
  
  va_end(ptr);
}

//----------------------------------------------------------------------------------------

void eclogger_err (EcLogLevel lvl, const char* unit, const char* method, int errcode, const char* format, ...)
{
  char buffer1 [1002];
  char buffer2 [1002];

  va_list ptr;

  va_start(ptr, format);

#ifdef _WIN32
  vsnprintf_s (buffer1, 1001, 1000, format, ptr );
  {
    LPSTR pBuffer = NULL;
    // use ansi version
    DWORD res = FormatMessageA (FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER, NULL, errcode, 0, (LPSTR)&pBuffer, 0, NULL);    
    if(res != 0)
    {
	    _snprintf_s (buffer2, 1001, 1000, "%s : '%s'", buffer1, pBuffer);
    }
    else
    {
      _snprintf_s (buffer2, 1001, 1000, "%s : [error fetching error message]", buffer1);
    }
    
    LocalFree(pBuffer);
  }
#elif __DOS__
  vsprintf((char*)self->buffer02->buffer, format, ptr);
  ecbuf_format (self->buffer01, 300, "%s : 'system error'", ecbuf_const_str (self->buffer02));
#else  
  vsnprintf (buffer1, 1000, format, ptr);
  snprintf (buffer2, 1000, "%s : '%s'", buffer1, strerror(errcode));
#endif  

  eclogger_msg (lvl, unit, method, buffer2);

  va_end(ptr);  
}

//----------------------------------------------------------------------------------------

void eclogger_errno (EcLogLevel lvl, const char* unit, const char* method, const char* format, ...)
{
  char buffer1 [1002];
  char buffer2 [1002];

  va_list ptr;

  va_start(ptr, format);

#ifdef _WIN32
  vsnprintf_s (buffer1, 1001, 1000, format, ptr );
  {
    LPSTR pBuffer = NULL;
    // use ansi version
    DWORD res = FormatMessageA (FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER, NULL, GetLastError (), 0, (LPSTR)&pBuffer, 0, NULL);    
    if(res != 0)
    {
	    _snprintf_s (buffer2, 1001, 1000, "%s : '%s'", buffer1, pBuffer);
    }
    else
    {
      _snprintf_s (buffer2, 1001, 1000, "%s : [error fetching error message]", buffer1);
    }
    
    LocalFree(pBuffer);
  }
#elif __DOS__
  vsprintf((char*)self->buffer02->buffer, format, ptr);
  ecbuf_format (self->buffer01, 300, "%s : 'system error'", ecbuf_const_str (self->buffer02));
#else  
  vsnprintf (buffer1, 1000, format, ptr);
  snprintf (buffer2, 1000, "%s : '%s'", buffer1, strerror(errno));
#endif  

  eclogger_msg (lvl, unit, method, buffer2);

  va_end(ptr);
}

//----------------------------------------------------------------------------------------


