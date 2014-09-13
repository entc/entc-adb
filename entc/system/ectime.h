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

#ifndef ENTC_SYSTEM_TIME_H
#define ENTC_SYSTEM_TIME_H 1

#include <system/macros.h>
#include <system/types.h>

#include <time.h>

typedef struct {
  
  uint_t usec;
  uint_t msec;
  uint_t sec;
  
} EcTime;

typedef struct {

  uint_t day;
  uint_t month;
  uint_t year;
  
  uint_t hour;
  uint_t minute;
  
  uint_t usec;
  uint_t msec;
  uint_t sec;
  
} EcDate;

__CPP_EXTERN______________________________________________________________________________START

__LIB_EXPORT void ectime_getTime (EcTime*);

__LIB_EXPORT void ectime_getDate (EcDate*);

__LIB_EXPORT void ectime_toTimeInfo (struct tm*, const time_t*);

__CPP_EXTERN______________________________________________________________________________END

#endif
