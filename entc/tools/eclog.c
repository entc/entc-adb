#include "eclog.h"

#include "types/ecerr.h"
#include "types/ecstring.h"
#include "types/ecbuffer.h"

#ifdef _WIN32

#include <windows.h>
#include <stdarg.h>
#include <stdio.h>

#else

#include <sys/time.h>
#include <stdarg.h>
#include <stdio.h>

#endif

//-----------------------------------------------------------------------------

static const char* msg_matrix[7] = { "___", "FAT", "ERR", "WRN", "INF", "DBG", "TRA" };
#if defined _WIN64 || defined _WIN32

#include <windows.h>

static WORD clr_matrix[11] = {
  0, 
  FOREGROUND_RED | FOREGROUND_INTENSITY,
  FOREGROUND_RED | FOREGROUND_INTENSITY,
  FOREGROUND_RED | FOREGROUND_BLUE | FOREGROUND_INTENSITY,
  FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_RED | FOREGROUND_INTENSITY,
  FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_RED,
  FOREGROUND_GREEN | FOREGROUND_BLUE
};

#else

static const char* clr_matrix[7] = { "0", "0;31", "1;31", "1;33", "0;32", "0;34", "0;30" };

#endif

//-----------------------------------------------------------------------------

void eclog_msg (EcLogLevel lvl, const char* unit, const char* method, const char* msg)
{
  static char buffer [2050];

  if (lvl > 6)
  {
    return;
  }
  
#if defined _WIN64 || defined _WIN32 

      _snprintf_s (buffer, 2048, _TRUNCATE, "%-12s %s|%-8s] %s", method, msg_matrix[lvl], unit, msg);
      {
        CONSOLE_SCREEN_BUFFER_INFO info;
        // get the console handle
        HANDLE hStdout = GetStdHandle (STD_OUTPUT_HANDLE);      
        // remember the original background color
        GetConsoleScreenBufferInfo (hStdout, &info);
        // do some fancy stuff
        SetConsoleTextAttribute (hStdout, clr_matrix[lvl]);

        printf("%s\n", buffer);

        SetConsoleTextAttribute (hStdout, info.wAttributes);
      }
#else
      snprintf (buffer, 2048, "%-12s %s|%-8s] %s", method, msg_matrix[lvl], unit, msg);

      printf("\033[%sm%s\033[0m\n", clr_matrix[lvl], buffer);
#endif  
}

//-----------------------------------------------------------------------------

void eclog_fmt (EcLogLevel lvl, const char* unit, const char* method, const char* format, ...)
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

  eclog_msg (lvl, unit, method, buffer);
  
  va_end(ptr);  
}

//-----------------------------------------------------------------------------

void eclog_bin (EcLogLevel lvl, const char* unit, const char* method, const char* data, uint64_t size)
{
  EcBuffer_s h = {(unsigned char*)data, size};
  EcBuffer hex = ecbuf_bin2hex (&h);
  
  EcString hexs = ecbuf_str (&hex);
  
  eclog_msg (lvl, unit, method, hexs);
  
  ecstr_delete (&hexs);  
}

//-----------------------------------------------------------------------------

void eclog_err_os (EcLogLevel lvl, const char* unit, const char* method, const char* format, ...)
{
  char buffer [1002];

  // variables
  va_list ptr;

  EcErr err = ecerr_create ();
  
  // retrieve the OS error text
  ecerr_lastErrorOS (err, ENTC_LVL_ERROR);

  va_start(ptr, format);

#ifdef _WIN32
  vsnprintf_s (buffer, 1001, 1000, format, ptr);
#else
  vsnprintf (buffer, 1000, format, ptr);
#endif 

  {
    EcString h = ecstr_cat3 (err->text, ": ", buffer);

    eclog_msg (lvl, unit, method, h);
  
    ecstr_delete (&h);
  }
   
  ecerr_destroy (&err);

  va_end(ptr);  
}

//-----------------------------------------------------------------------------
