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

#ifdef _WIN32
#include <windows.h>
#else
#include <sys/time.h>
#endif

//----------------------------------------------------------------------------------------

static EcEchoLogger toggleEchoLogger (int role) {  
  static int counter = 0;
  static EcEchoLogger logger = NULL;
  
  if (role)
  {
    if (counter == 0)
    {
      logger = ecechologger_new ();
    }
    counter++;
  }
  else
  {
    counter--;
    if (counter == 0)
    {
      ecechologger_del(&logger);
    }
  }
  return logger;
}

#endif

//----------------------------------------------------------------------------------------

struct EcLogger_s
{

  EcMutex mutex;

  EcLoggerCallbacks callbacks;
  
  ubyte_t threadid;
  /* the temp buffer maximal 300 characters */
  /* only needed for formatted logs */
  EcBuffer buffer01;
  EcBuffer buffer02;
  
  EcList errmsgs;

  EcList sccmsgs;
  
  ubyte_t sec;

};

//----------------------------------------------------------------------------------------

EcLogger eclogger_new (ubyte_t threadid)
{
  EcEchoLogger echo;

  EcLogger self = ENTC_NEW(struct EcLogger_s);
  
  self->mutex = ecmutex_new ();
  
  self->threadid = threadid;
  
  self->sec = 0;
  
  self->errmsgs = eclist_new();
  self->sccmsgs = eclist_new();

  echo = toggleEchoLogger (TRUE);

  ecechologger_getCallback(echo, &(self->callbacks));  
    
  self->buffer01 = ecstr_buffer(1024);
  self->buffer02 = ecstr_buffer(1024);

  return self;
}

//----------------------------------------------------------------------------------------

void eclogger_del (EcLogger* pself)
{  
  EcLogger self = *pself;
  
  eclogger_clear(self);

  eclist_delete(&(self->errmsgs));
  eclist_delete(&(self->sccmsgs));

  ecstr_release(&(self->buffer01));
  ecstr_release(&(self->buffer02));

  toggleEchoLogger (FALSE);
  
  ecmutex_delete(&(self->mutex));
  
  ENTC_DEL(pself, struct EcLogger_s);
}

//----------------------------------------------------------------------------------------

void eclogger_sync (EcLogger self, const EcLogger other)
{
  ecmutex_lock (self->mutex);  
  memcpy (&(self->callbacks), &(other->callbacks), sizeof(EcLoggerCallbacks));
  ecmutex_unlock (self->mutex);
}

//----------------------------------------------------------------------------------------

void eclogger_setCallback (EcLogger self, EcLoggerCallbacks* callbacks)
{
  ecmutex_lock (self->mutex);  
  memcpy (&(self->callbacks), callbacks, sizeof(EcLoggerCallbacks));
  ecmutex_unlock (self->mutex);
}

//----------------------------------------------------------------------------------------

void eclogger_getCallback (EcLogger self, EcLoggerCallbacks* callbacks)
{
  ecmutex_lock (self->mutex);  
  memcpy (callbacks, &(self->callbacks), sizeof(EcLoggerCallbacks));
  ecmutex_unlock (self->mutex);
}

//----------------------------------------------------------------------------------------

void eclogger_setLogLevel (EcLogger self, EcLogLevel level)
{
  
}

//----------------------------------------------------------------------------------------

void eclogger_logthsafe (EcLogger self, EcLogLevel level, const EcString module, const EcString msg)
{
  // here everything is already thread safe
  if (isAssigned (self->callbacks.logfct) && (level >= LL_FATAL) && (level <= LL_TRACE))
  {
    self->callbacks.logfct (self->callbacks.ptr, level, self->threadid, module, msg);
  }
}

//----------------------------------------------------------------------------------------

void eclogger_log (EcLogger self, EcLogLevel level, const char* module, const char* msg)
{
  // check if valid (this is the specification)
  if (isNotAssigned(self))
  {
    return;  
  }
  // wrap method in monitor
  
  ecmutex_lock (self->mutex);  
  
  eclogger_logthsafe (self, level, module, msg);
  
  ecmutex_unlock (self->mutex);
}

//----------------------------------------------------------------------------------------

void eclogger_logformat (EcLogger self, EcLogLevel level, const char* module, const char* format, ...)
{
  // variables
  va_list ptr;
  // check if valid (this is the specification)
  if (isNotAssigned(self))
  {
    return;  
  }
  
  ecmutex_lock (self->mutex);  
  
  va_start(ptr, format);
  
#ifdef _WIN32
  vsnprintf_s((char*)self->buffer01->buffer, self->buffer01->size, self->buffer01->size - 1, format, ptr );    
#elif __DOS__
  vsprintf((char*)self->buffer01->buffer, format, ptr);        
#else
  vsnprintf((char*)self->buffer01->buffer, self->buffer01->size, format, ptr );
#endif
  eclogger_logthsafe (self, level, (const EcString)module, ecstr_get(self->buffer01));
  
  va_end(ptr);
  
  ecmutex_unlock (self->mutex);  
}

//----------------------------------------------------------------------------------------

void eclogger_logerr (EcLogger self, EcLogLevel level, const char* module, int error, const char* format, ...)
{
  // variables
  va_list ptr;
  // check if valid (this is the specification)
  if (isNotAssigned(self))
  {
    return;  
  }

  ecmutex_lock (self->mutex);  

  va_start(ptr, format);
  
#ifdef _WIN32
  vsnprintf_s((char*)self->buffer02->buffer, self->buffer02->size, self->buffer02->size - 1, format, ptr );
  {
    LPSTR pBuffer = NULL;
    // use ansi version
    DWORD res = FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER, NULL, error, 0, (LPSTR)&pBuffer, 0, NULL);    
    if(res != 0)
    {
	  ecstr_format(self->buffer01, self->buffer01->size, "%s : '%s'", ecstr_get(self->buffer02), pBuffer);
    }
    else
    {
      ecstr_format(self->buffer01, self->buffer01->size, "%s : [error fetching error message]", ecstr_get(self->buffer02));
    }
    
    LocalFree(pBuffer);
  }
#elif __DOS__
  vsprintf((char*)self->buffer02->buffer, format, ptr);
  ecstr_format(self->buffer01, 300, "%s : 'system error'", ecstr_get(self->buffer02));
#else  
  vsnprintf((char*)self->buffer02->buffer, self->buffer02->size, format, ptr );
  ecstr_format(self->buffer01, 300, "%s : '%s'", ecstr_get(self->buffer02), strerror(error));
#endif
  
  eclogger_logthsafe (self, level, (const EcString)module, ecstr_get(self->buffer01));
  
  va_end(ptr); 
  
  ecmutex_unlock (self->mutex);  
}

//----------------------------------------------------------------------------------------

void eclogger_logerrno (EcLogger self, EcLogLevel level, const char* module, const char* format, ...)
{
  // variables
  va_list ptr;
  // check if valid (this is the specification)
  if (isNotAssigned(self))
  {
    return;  
  }

  ecmutex_lock (self->mutex);  

  va_start(ptr, format);

#ifdef _WIN32
  vsnprintf_s((char*)self->buffer02->buffer, self->buffer02->size, self->buffer02->size - 1, format, ptr );
  {
    LPSTR pBuffer = NULL;
    
    DWORD res = FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER, NULL, GetLastError(), 0, (LPSTR)&pBuffer, 0, NULL);    
    if(res != 0)
	{
      ecstr_format(self->buffer01, self->buffer01->size, "%s : '%s'", ecstr_get(self->buffer02), pBuffer);
	}
	else
	{
      ecstr_format(self->buffer01, self->buffer01->size, "%s : [error fetching error message]", ecstr_get(self->buffer02));
	}

    LocalFree(pBuffer);
  }
#elif __DOS__
  
#else  
  vsnprintf((char*)self->buffer02->buffer, self->buffer02->size, format, ptr );
  ecstr_format(self->buffer01, 300, "%s : '%s'", ecstr_get(self->buffer02), strerror(errno));
#endif

  eclogger_logthsafe (self, level, (const EcString)module, ecstr_get(self->buffer01));

  va_end(ptr);

  ecmutex_unlock (self->mutex);  
}

//----------------------------------------------------------------------------------------

void eclogger_logbinary (EcLogger self, EcLogLevel level, const char* module, const char* data, uint_t size)
{
  // variables
  const char* pos01 = data;
  unsigned char* pos02 = self->buffer01->buffer;
  uint_t i;
  // check if valid (this is the specification)
  if (isNotAssigned(self))
  {
    return;  
  }

  ecmutex_lock (self->mutex);  

  /* convert to readable format */
  for(i = 0; (i < size) && (pos02 < (self->buffer01->buffer + self->buffer01->size - 3)); i++ )
  {
    unsigned char c = *pos01;
    
    if(c < 10)
    {
      *pos02 = '#';
      pos02++;
      
      *pos02 = '0';
      pos02++;

      *pos02 = c + 48;
      pos02++;
    }
    else if( c < 32 )
    {
      *pos02 = '[';
      pos02++;
      *pos02 = '#';
      pos02++;
#ifdef _WIN32
	  _snprintf_s((char*)pos02, 1, 1, "%c", c);
#elif __DOS__
    sprintf((char*)pos02, "%c", c);
#else
	  snprintf((char*)pos02, 1, "%c", c);
#endif
      pos02 += 2;
      *pos02 = ']';
      pos02++;      
    }
    else
    {
      *pos02 = c;
      pos02++;      
    }
      
    pos01++;
    
  }
  /* terminate */
  *pos02 = 0;
  
  eclogger_logthsafe (self, level, (const EcString)module, ecstr_get(self->buffer01));

  ecmutex_unlock (self->mutex);  
}

//----------------------------------------------------------------------------------------

EcUdc eclogger_message (EcLogger self, uint_t logid, uint_t messageid, EcUdc* data)
{
  //variables
  EcLoggerCallbacks copy;
  // check if valid (this is the specification)
  if (isNotAssigned(self))
  {
    return NULL;  
  }
  
  ecmutex_lock (self->mutex);  

  memcpy(&copy, &(self->callbacks), sizeof(EcLoggerCallbacks));
  
  ecmutex_unlock (self->mutex);  

  if (isAssigned (copy.msgfct))
  {
    return copy.msgfct (copy.ptr, logid, messageid, data);
  }

  return NULL;  
}

//----------------------------------------------------------------------------------------

EcList eclogger_errmsgs (EcLogger self)
{
  return self->errmsgs;
}

//----------------------------------------------------------------------------------------

EcList eclogger_sccmsgs (EcLogger self)
{
  return self->sccmsgs;  
}

//----------------------------------------------------------------------------------------

void eclogger_clear (EcLogger self)
{
  EcListNode node;
  
  ecmutex_lock (self->mutex);  
  
  for(node = eclist_first(self->errmsgs); node != eclist_end(self->errmsgs); node = eclist_next(node))
    free(eclist_data(node));
  
  eclist_clear(self->errmsgs);

  for(node = eclist_first(self->sccmsgs); node != eclist_end(self->sccmsgs); node = eclist_next(node))
    free(eclist_data(node));

  eclist_clear(self->sccmsgs);
  
  self->sec = 0;
  
  ecmutex_unlock (self->mutex); 
}

//----------------------------------------------------------------------------------------

void eclogger_sec (EcLogger self, EcSecLevel sec)
{
  self->sec = sec;
}

//----------------------------------------------------------------------------------------

uint_t eclogger_securityIncidents(EcLogger self)
{
  return self->sec;
}

//----------------------------------------------------------------------------------------

static const char* msg_matrix[11] = { "___", "FAT", "ERR", "WRN", "INF", "DBG", "TRA" };
#if defined _WIN64 || defined _WIN32
static WORD clr_matrix[11] = {0, FOREGROUND_GREEN | FOREGROUND_BLUE, FOREGROUND_GREEN | FOREGROUND_BLUE, FOREGROUND_BLUE, FOREGROUND_RED | FOREGROUND_BLUE, 0, FOREGROUND_RED | FOREGROUND_GREEN};
#else
static const char* clr_matrix[11] = { "0", "0;31", "1;31", "1;33", "0;32", "0;30", "1;34" };
#endif
static const char* month_matrix[12] = {"01","02","03","04","05","06","07","08","09","10","11","12"};

//----------------------------------------------------------------------------------------

struct EcEchoLogger_s 
{

  EcMutex mutex;
  
  EcBuffer buffer01;

  EcBuffer buffer02;

#if defined _WIN64 || defined _WIN32
  HANDLE hConsole;
  CONSOLE_SCREEN_BUFFER_INFO pInfo;
#endif

};

//----------------------------------------------------------------------------------------

EcEchoLogger ecechologger_new ()
{
  EcEchoLogger self = ENTC_NEW (struct EcEchoLogger_s);
  
  self->mutex = ecmutex_new ();
  self->buffer01 = ecstr_buffer(14);
  self->buffer02 = ecstr_buffer(201);

#if defined _WIN64 || defined _WIN32
  self->hConsole = GetStdHandle (STD_OUTPUT_HANDLE);
  GetConsoleScreenBufferInfo(self->hConsole, &(self->pInfo));
#endif

  return self;  
}

//----------------------------------------------------------------------------------------

void ecechologger_del (EcEchoLogger* pself)
{
  EcEchoLogger self = *pself;
  
  ecstr_release(&(self->buffer01));
  ecstr_release(&(self->buffer02));
  ecmutex_delete(&(self->mutex));
  
  ENTC_DEL (pself, struct EcEchoLogger_s);    
}

//----------------------------------------------------------------------------------------

void ecechologger_log (void* ptr, EcLogLevel level, ubyte_t id, const EcString module, const EcString msg)
{
  // cast
  EcEchoLogger self = ptr;
  // variables
  EcDate date;
  
  ectime_getDate (&date);
  
  // ***** monitor start *****************************************************
  ecmutex_lock (self->mutex);

  ecstr_format (self->buffer02, 200, "%04u-%s-%02u %02u:%02u:%02u.%03u", date.year, month_matrix[date.month], date.day, date.hour, date.minute, date.sec, date.msec);
  
  // prepare the message
  ecstr_format (self->buffer01, 11, "%s %4s ", msg_matrix[level], module );
  
#if defined _WIN64 || defined _WIN32 
  SetConsoleTextAttribute(self->hConsole, self->pInfo.wAttributes);
  printf("%s %i %s%s\n", self->buffer02->buffer, id, self->buffer01->buffer, msg);
#else
  // use color theme for different levels
  printf("%s \033[%sm%02i %s%s\033[0m\n", self->buffer02->buffer, clr_matrix[level], id, self->buffer01->buffer, msg);
#endif

  ecmutex_unlock (self->mutex);
  // ***** monitor end *******************************************************
}

//----------------------------------------------------------------------------------------

void ecechologger_getCallback (EcEchoLogger self, EcLoggerCallbacks* callbacks)
{
  callbacks->logfct = ecechologger_log;
  callbacks->msgfct = NULL;
  callbacks->ptr = self;
}

//----------------------------------------------------------------------------------------

struct EcFileLogger_s 
{

  EcMutex mutex;
  
  EcBuffer buffer01;
  
  EcBuffer buffer02;

  EcString filename;
  
  EcFileHandle fhandle;
  
};

//----------------------------------------------------------------------------------------

EcFileLogger ecfilelogger_new (const EcString filename)
{
  EcFileLogger self = ENTC_NEW (struct EcFileLogger_s);
  
  self->mutex = ecmutex_new ();
  self->buffer01 = ecstr_buffer(201);
  self->buffer02 = ecstr_buffer(17);  
  
  self->filename = ecstr_copy(filename);
  self->fhandle = NULL;
  
  return self;  
}

//----------------------------------------------------------------------------------------

void ecfilelogger_del (EcFileLogger* pself)
{
  EcFileLogger self = *pself;
  
  if (isAssigned (self->fhandle))
  {
    ecfh_close(&(self->fhandle));
  }
  
  ecstr_delete(&(self->filename));
  
  ecmutex_delete(&(self->mutex));
  ecstr_release(&(self->buffer01));
  ecstr_release(&(self->buffer02));
  
  ENTC_DEL (pself, struct EcFileLogger_s);  
}

//----------------------------------------------------------------------------------------

void ecfilelogger_log (void* ptr, EcLogLevel level, ubyte_t id, const EcString module, const EcString msg)
{
  // cast
  EcFileLogger self = ptr;
  // variables
  EcDate date;
  
  if (isNotAssigned(self->fhandle))
  {
    printf("try to open '%s'\n", self->filename);
    
    self->fhandle = ecfh_open(self->filename, O_WRONLY | O_APPEND | O_CREAT);
  }
  if (isNotAssigned(self->fhandle))
  {
    return;
  }
  
  ectime_getDate (&date);
  
  // ***** monitor start *****************************************************
  ecmutex_lock (self->mutex);
  
  ecstr_format (self->buffer01, 200, "%04u-%s-%02u %02u:%02u:%02u.%03u", date.year, month_matrix[date.month], date.day, date.hour, date.minute, date.sec, date.msec);
  
  // prepare the message
  ecstr_format (self->buffer02, 16, " %02i %s %4s ", id, msg_matrix[level], module );

  ecfh_writeBuffer (self->fhandle, self->buffer01, 200);
  ecfh_writeBuffer (self->fhandle, self->buffer02, 16);
  ecfh_writeString (self->fhandle, msg);
  ecfh_writeString (self->fhandle, "\n");
  
  ecmutex_unlock (self->mutex);
  // ***** monitor end *******************************************************
}

//----------------------------------------------------------------------------------------

void ecfilelogger_getCallback (EcFileLogger self, EcLoggerCallbacks* callbacks)
{
  callbacks->logfct = ecfilelogger_log;
  callbacks->msgfct = NULL;
  callbacks->ptr = self;
}

//----------------------------------------------------------------------------------------

struct EcListLogger_s
{

  EcReadWriteLock rwlock;

  EcList list;
  
  EcList shadow_list;
  
  int list_changed;

};

//----------------------------------------------------------------------------------------

EcListLogger eclistlogger_new ()
{
  EcListLogger self = ENTC_NEW (struct EcListLogger_s);
  
  self->rwlock = ecreadwritelock_new ();
  self->list = eclist_new ();
  self->shadow_list = eclist_new ();
  
  self->list_changed = FALSE;
  
  return self;
}

//----------------------------------------------------------------------------------------

void eclistlogger_clear (EcList self)
{
  EcListNode node;

  for (node = eclist_first (self); node != eclist_end (self); node = eclist_next (node))
  {
    EcLoggerCallbacks* callbacks = eclist_data(node);

    ENTC_DEL (&callbacks, EcLoggerCallbacks);
  }
  
  eclist_clear(self);
}

//----------------------------------------------------------------------------------------

void eclistlogger_del (EcListLogger* pself)
{
  EcListLogger self = *pself;
  
  eclistlogger_clear (self->list);
  eclistlogger_clear (self->shadow_list);
  
  eclist_delete(&(self->list));
  eclist_delete(&(self->shadow_list));

  ecreadwritelock_delete(&(self->rwlock));
  
  ENTC_DEL (pself, struct EcListLogger_s);
}

//----------------------------------------------------------------------------------------

void eclistlogger_sync (EcListLogger self)
{
  EcListNode node;

  if (self->list_changed)
  {
    eclistlogger_clear (self->list);
    
    // copy back the whole list
    for (node = eclist_first (self->shadow_list); node != eclist_end (self->shadow_list); node = eclist_next (node))
    {
      const EcLoggerCallbacks* callbacks = eclist_data(node);
      
      EcLoggerCallbacks* tmp1 = ENTC_NEW (EcLoggerCallbacks);
      
      memcpy (tmp1, callbacks, sizeof(EcLoggerCallbacks));
      
      eclist_append(self->list, tmp1);
    }
    
    self->list_changed = FALSE;
  }  
}

//----------------------------------------------------------------------------------------

void eclistlogger_log (void* ptr, EcLogLevel level, ubyte_t id, const EcString module, const EcString msg)
{
  EcListLogger self = ptr;
  
  EcListNode node;
  
  if (ecreadwritelock_lockReadAndTransformIfFirst (self->rwlock))
  {
    eclistlogger_sync (self);    
    ecreadwritelock_unlockWrite (self->rwlock);
    ecreadwritelock_lockRead (self->rwlock);
  }
  
  for (node = eclist_first (self->list); node != eclist_end (self->list); node = eclist_next (node))
  {
    const EcLoggerCallbacks* callbacks = eclist_data(node);
    
    if (isAssigned (callbacks->logfct))
    {
      callbacks->logfct (callbacks->ptr, level, id, module, msg);
    }
  }

  if (ecreadwritelock_unlockReadAndTransformIfLast (self->rwlock))
  {
    eclistlogger_sync (self);
    ecreadwritelock_unlockWrite (self->rwlock);          
  }
}

//----------------------------------------------------------------------------------------

EcUdc eclistlogger_message (void* ptr, uint_t logid, uint_t messageid, EcUdc* data)
{
  // cast
  EcListLogger self = ptr;
  // variables
  EcListNode node;
  EcUdc ret = NULL;
  
  if (ecreadwritelock_lockReadAndTransformIfFirst (self->rwlock))
  {
    eclistlogger_sync (self);    
    ecreadwritelock_unlockWrite (self->rwlock);
    ecreadwritelock_lockRead (self->rwlock);
  }

  for (node = eclist_first (self->list); node != eclist_end (self->list); node = eclist_next (node))
  {
    const EcLoggerCallbacks* callbacks = eclist_data(node);
    
    if (isAssigned (callbacks->msgfct))
    {
      if ( isNotAssigned(ret))
      {
        // create a new udc node
        ret = ecudc_new(ENTC_UDC_NODE, "ServiceResults");        
      }
      {
        EcUdc res = callbacks->msgfct (callbacks->ptr, logid, messageid, data);
        if (isAssigned (res))
        {
          // transfer the object to the udc node
          ecudc_add(ret, &res);
        }
      }
    }
  }
  
  if (ecreadwritelock_unlockReadAndTransformIfLast (self->rwlock))
  {
    eclistlogger_sync (self);
    ecreadwritelock_unlockWrite (self->rwlock);          
  }

  return ret;
}

//----------------------------------------------------------------------------------------

void eclistlogger_getCallback (EcListLogger self, EcLoggerCallbacks* callbacks)
{
  callbacks->logfct = eclistlogger_log;
  callbacks->msgfct = eclistlogger_message;
  callbacks->ptr = self;
}

//----------------------------------------------------------------------------------------

void eclistlogger_register (EcListLogger self, EcLogger logger)
{
  EcLoggerCallbacks* tmp1 = ENTC_NEW (EcLoggerCallbacks);
  EcLoggerCallbacks tmp2;

  eclogger_getCallback(logger, tmp1);
  
  eclistlogger_getCallback(self, &tmp2);
  
  eclogger_setCallback(logger, &tmp2);
  
  ecreadwritelock_lockWrite (self->rwlock);
  
  eclist_append(self->shadow_list, tmp1);
  
  self->list_changed = TRUE;
  
  ecreadwritelock_unlockWrite (self->rwlock);
}

//----------------------------------------------------------------------------------------

EcListNode eclistlogger_add (EcListLogger self, const EcLoggerCallbacks* callbacks)
{
  EcListNode ret;
  
  EcLoggerCallbacks* tmp1 = ENTC_NEW (EcLoggerCallbacks);

  memcpy (tmp1, callbacks, sizeof(EcLoggerCallbacks));
  
  ret = eclist_append(self->shadow_list, tmp1);  

  self->list_changed = TRUE;
  
  return ret;
}

//----------------------------------------------------------------------------------------

void eclistlogger_remove (EcListLogger self, EcListNode node)
{
  EcLoggerCallbacks* callbacks = eclist_data(node);
  
  ENTC_DEL (&callbacks, EcLoggerCallbacks);
  
  eclist_erase(node);
  
  self->list_changed = TRUE;
}

//----------------------------------------------------------------------------------------


