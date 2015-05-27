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

#include "ectime.h"

#include "utils/eclogger.h"
#include <errno.h>
#include <stdio.h>

#if defined _WIN64 || defined _WIN32

#include <windows.h>

#elif __APPLE_CC__

#include <sys/time.h>

#else

#include <time.h>

#endif

//-----------------------------------------------------------------------------------

void ectime_getTime (EcTime* ectime)
{

#if defined _WIN64 || defined _WIN32

  SYSTEMTIME time;
  GetSystemTime(&time);
  
  ectime->sec = time.wSecond;
  ectime->msec = time.wMilliseconds;
  ectime->usec = 0;

#else

  struct timeval time;  
  gettimeofday (&time, NULL);
  
  ectime->sec = time.tv_sec;
  ectime->msec = time.tv_usec / 1000;
  ectime->usec = time.tv_usec;
  
#endif
  
}

//-----------------------------------------------------------------------------------

void ectime_getDate (EcDate* ecdate)
{

#if defined _WIN64 || defined _WIN32
  
  SYSTEMTIME time;
  GetSystemTime(&time);
  
  ecdate->sec = time.wSecond;
  ecdate->msec = time.wMilliseconds;
  ecdate->usec = 0;
  
  ecdate->minute = time.wMinute;
  ecdate->hour = time.wHour;
  
  ecdate->day = time.wDay;
  ecdate->month = time.wMonth;
  ecdate->year = time.wYear;
  
#else
  
  struct timeval time;  
  struct tm* l01;

  gettimeofday (&time, NULL);
  l01 = localtime (&(time.tv_sec));
  
  ecdate->sec = l01->tm_sec;
  ecdate->msec = time.tv_usec / 1000;
  ecdate->usec = time.tv_usec;

  ecdate->minute = l01->tm_min;
  ecdate->hour = l01->tm_hour;
  
  ecdate->day = l01->tm_mday;
  ecdate->month = l01->tm_mon;
  ecdate->year = l01->tm_year + 1900;
  
#endif
  
}

//-----------------------------------------------------------------------------------

void ectime_toTimeInfo (struct tm* timeinfo, const time_t* t)
{
#if defined _WIN64 || defined _WIN32
  gmtime_s (timeinfo, t);
#else
  gmtime_r (t, timeinfo);
#endif
}

//-----------------------------------------------------------------------------------

void ectime_toGmtString (const time_t* t, char* buffer, ulong_t size)
{
  struct tm timeinfo;

  // fill the timeinfo
  ectime_toTimeInfo (&timeinfo, t);

  // create buffer with timeinfo as string
  strftime (buffer, size, "%a, %d %b %Y %H:%M:%S GMT", &timeinfo);
}

//-----------------------------------------------------------------------------------

void ectime_toISO8601 (const time_t* t, char* buffer, ulong_t size)
{
  struct tm timeinfo;
  
  // fill the timeinfo
  ectime_toTimeInfo (&timeinfo, t);
  
  // create buffer with timeinfo as string
  strftime (buffer, size, "%Y%m%dT%H%M%SZ", &timeinfo);
}

//-----------------------------------------------------------------------------------

void ectime_parseISO8601 (time_t* t, const char* stime)
{
  struct tm timeinfo;  
  int year, month, day, hour, minute, second;
  int res = sscanf (stime, "%4d%2d%2dT%2d%2d%2d", &year, &month, &day, &hour, &minute, &second);

  if (res != 6)
  {
    eclogger_fmt (LL_ERROR, "ENTC", "parse iso8601", "can't parse time string '%s', %i", stime, res);
    return;
  }

  timeinfo.tm_year = year - 1900;
  timeinfo.tm_mon = month - 1;
  timeinfo.tm_mday = day;

  timeinfo.tm_hour = hour;
  timeinfo.tm_min = minute;
  timeinfo.tm_sec = second;

  timeinfo.tm_isdst = 1;
  
  *t = mktime (&timeinfo);
}

//-----------------------------------------------------------------------------------
