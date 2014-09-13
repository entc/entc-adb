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
