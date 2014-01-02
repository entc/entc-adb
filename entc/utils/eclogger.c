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

static EcMutex global_mutex = NULL;

static void globalMutex(int role) {  
  static int counter = 0;
  
  if (role)
  {
    if (counter == 0)
    {
      global_mutex = ecmutex_new();  
    }
    counter++;
  }
  else
  {
    counter--;
    if (counter == 0)
    {
      ecmutex_delete(&global_mutex);     
      
    }
  }
}

#endif

struct EcLogger_s
{
  
  ubyte_t threadid;
  /* the temp buffer maximal 300 characters */
  /* only needed for formatted logs */
  EcBuffer buffer01;
  EcBuffer buffer02;
  EcBuffer buffer03;
  
#ifndef __DOS__

  EcString logfile;
  
  
  EcFileHandle fhandle;
  
  EcList errmsgs;

  EcList sccmsgs;
  
  ubyte_t sec;

#endif
  
};

/*------------------------------------------------------------------------*/

EcLogger eclogger_new (ubyte_t threadid)
{
  EcLogger self = ENTC_NEW(struct EcLogger_s);
  
  self->threadid = threadid;
  
#ifndef __DOS__
  
  self->logfile = 0;
  self->fhandle = NULL;
  self->sec = 0;
  
  self->errmsgs = eclist_new();
  self->sccmsgs = eclist_new();

  globalMutex(TRUE);

#endif
  
  self->buffer01 = ecstr_buffer(1024);
  self->buffer02 = ecstr_buffer(1024);
  self->buffer03 = ecstr_buffer(14);

  return self;
}

/*------------------------------------------------------------------------*/

void eclogger_delete(EcLogger* ptr_self)
{  
  EcLogger self = *ptr_self;
  
#ifndef __DOS__
  
  if (self->fhandle)
  {
    ecfh_close(&(self->fhandle));
  }
    
  eclogger_clear(self);

  eclist_delete(&(self->errmsgs));
  eclist_delete(&(self->sccmsgs));

  ecstr_delete(&(self->logfile));

  globalMutex(FALSE);

#endif
  
  ecstr_release(&(self->buffer01));
  ecstr_release(&(self->buffer02));
  ecstr_release(&(self->buffer03));

  ENTC_DEL(ptr_self, struct EcLogger_s);
}

/*------------------------------------------------------------------------*/

#ifndef __DOS__

void eclogger_paramLogFile(EcLogger self, const EcString confdir, int argc, char *argv[])
{
  EcString value = ecstr_extractParameter('l', argc, argv);
  if( ecstr_valid(value) )
  {
    EcString logfile = ecfs_mergeToPath(confdir, value);
    
    eclogger_setLogFile(self, logfile, confdir);
    
    ecstr_delete(&logfile);
  }
  ecstr_delete(&value);
}

#endif

/*------------------------------------------------------------------------*/

static const char* msg_matrix[11] = { "___", "FAT", "ERR", "WRN", "INF", "DBG", "TRA" };
static const char* clr_matrix[11] = { "0", "0;31", "1;31", "1;33", "0;32", "0;30", "1;34" };

/*------------------------------------------------------------------------*/

void eclogger_prepare_message(unsigned char mode, const EcString module, const EcString msg)
{
  
}

/*------------------------------------------------------------------------*/

void eclogger_sec (EcLogger self, EcSecLevel level)
{
  
}

//------------------------------------------------------------------------------------------------------

void eclogger_logmt (EcLogger self, EcLogLevel level, const EcString module, const EcString msg)
{
  static const char* month_matrix[12] = {"Jan","Feb","Mar","Apr","May","Jun","Jul","Aug","Sep","Oct","Nov","Dec"};

  EcString timestamp;
  EcDate date;  
  EcBuffer buffer = ecstr_buffer(201);
  
  ectime_getDate (&date);
  
  ecstr_format(buffer, 200, "%02u-%s-%04u %02u:%02u:%02u.%03u", date.day, month_matrix[date.month], date.year, date.hour, date.minute, date.sec, date.msec);
  
  timestamp = ecstr_trans(&buffer);
  
  /* prepare the message */
  /* transform timestamp to the buffer */
  /* this buffer has always a static length */
#ifdef __DOS__
  ecstr_format(self->buffer03, 11, "%s %4s ", msg_matrix[mode], module );
    
  printf("%s %i %s%s\n", timestamp, self->threadid, self->buffer03->buffer, msg);
    
#else
  
  if ((level < 1) || (level > 6)) 
  {
    return;
  }

  ecstr_format (self->buffer03, 11, "%s %4s ", msg_matrix[level], module );
  
  if(self->fhandle == 0)
  {
    
#if defined _WIN64 || defined _WIN32 
    printf("%s %02i %s%s\n", timestamp, self->threadid, self->buffer03->buffer, msg);
#else
    // use color theme for different levels
    printf("%s \033[%sm%02i %s%s\033[0m\n", timestamp, clr_matrix[level], self->threadid, self->buffer03->buffer, msg);
#endif
  }
  else
  {
    ecfh_writeString(self->fhandle, timestamp);
    ecfh_writeBuffer(self->fhandle, self->buffer03, 11);
    ecfh_writeString(self->fhandle, msg);
    ecfh_writeString(self->fhandle, "\n");    
  }
  ecstr_delete(&timestamp);

  if(level == LL_ERROR)
  {
    eclist_append(self->errmsgs, ecstr_copy(msg));
  }

#endif
}

/*------------------------------------------------------------------------*/

void eclogger_log(EcLogger self, EcLogLevel level, const char* module, const char* msg)
{
  /* do we have a valid logger object */
  if( !self )
  {
    return;  
  }

#ifndef __DOS__
  ecmutex_lock(global_mutex);
#endif
  
  eclogger_logmt(self, level, (const EcString)module, (const EcString)msg);

#ifndef __DOS__
  ecmutex_unlock(global_mutex);
#endif
}

/*------------------------------------------------------------------------*/

void eclogger_logformat (EcLogger self, EcLogLevel level, const char* module, const char* format, ...)
{
  /* variables */
  va_list ptr;
  /* do we have a valid logger object */
  if(self)
  {
#ifndef __DOS__
    ecmutex_lock(global_mutex);
#endif

    va_start(ptr, format);
    
#ifdef _WIN32
    vsnprintf_s((char*)self->buffer01->buffer, self->buffer01->size, self->buffer01->size - 1, format, ptr );    
#elif __DOS__
    vsprintf((char*)self->buffer01->buffer, format, ptr);        
#else
    vsnprintf((char*)self->buffer01->buffer, self->buffer01->size, format, ptr );
#endif
    eclogger_logmt(self, level, (const EcString)module, ecstr_get(self->buffer01));
    
    va_end(ptr);
    
#ifndef __DOS__
    ecmutex_unlock(global_mutex);  
#endif
  }
}

/*------------------------------------------------------------------------*/

void eclogger_logerr(EcLogger self, EcLogLevel level, const char* module, int error, const char* format, ...)
{
  va_list ptr;
  /* do we have a valid logger object */
  if( !self )
  {
    return;  
  }
#ifndef __DOS__  
  ecmutex_lock(global_mutex);
#endif
  
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
  
  eclogger_logmt(self, level, (const EcString)module, ecstr_get(self->buffer01));
  
  va_end(ptr);
  
#ifndef __DOS__  
  ecmutex_unlock(global_mutex);
#endif
}

/*------------------------------------------------------------------------*/

void eclogger_logerrno(EcLogger self, EcLogLevel level, const char* module, const char* format, ...)
{
  va_list ptr;
  /* do we have a valid logger object */
  if( !self )
  {
    return;  
  }
  
#ifndef __DOS__ 
  ecmutex_lock(global_mutex);
#endif

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

  eclogger_logmt (self, level, (const EcString)module, ecstr_get(self->buffer01));

  va_end(ptr);

#ifndef __DOS__ 
  ecmutex_unlock(global_mutex);
#endif
}

/*------------------------------------------------------------------------*/

void eclogger_logbinary(EcLogger self, EcLogLevel level, const char* module, const char* data, uint_t size)
{
  const char* pos01 = data;
  unsigned char* pos02 = self->buffer01->buffer;
  uint_t i;
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
  
  eclogger_logmt (self, level, (const EcString)module, ecstr_get(self->buffer01));
}

/*------------------------------------------------------------------------*/

#ifndef __DOS__ 

void eclogger_setLogFile (EcLogger self, const EcString logfile, const EcString confdir)
{
  /* variables */
  struct EcSecFopen secopen;
  if( !ecstr_valid(logfile) )
  {
    return;
  }
  /* try to open the file */
  if( !ecsec_fopen(&secopen, logfile, O_WRONLY | O_APPEND | O_CREAT, self, confdir) )
  {
    eclogger_logformat(self, LL_ERROR, "LOGS", "Logger can't open file '%s'", secopen.filename );

    return;
  }
  
  eclogger_logformat(self, LL_INFO, "LOGS", "Logger uses file '%s'", secopen.filename );
  
  self->logfile = secopen.filename;
  self->fhandle = secopen.fhandle;
}

/*------------------------------------------------------------------------*/

void eclogger_openPipe(EcLogger self)
{
  
}

/*------------------------------------------------------------------------*/

const char* eclogger_getLogFile(EcLogger self)
{
  return self->logfile;
}

/*------------------------------------------------------------------------*/

EcList eclogger_errmsgs(EcLogger self)
{
  return self->errmsgs;
}

/*------------------------------------------------------------------------*/

EcList eclogger_sccmsgs(EcLogger self)
{
  return self->sccmsgs;  
}

/*------------------------------------------------------------------------*/

void eclogger_clear(EcLogger self)
{
  EcListNode node;
  for(node = eclist_first(self->errmsgs); node != eclist_end(self->errmsgs); node = eclist_next(node))
    free(eclist_data(node));
  
  eclist_clear(self->errmsgs);

  for(node = eclist_first(self->sccmsgs); node != eclist_end(self->sccmsgs); node = eclist_next(node))
    free(eclist_data(node));

  eclist_clear(self->sccmsgs);
  
  self->sec = 0;
}

/*------------------------------------------------------------------------*/

uint_t eclogger_securityIncidents(EcLogger self)
{
  return self->sec;
}

/*------------------------------------------------------------------------*/

#endif
