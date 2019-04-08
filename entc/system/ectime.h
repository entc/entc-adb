/*
 * Copyright (c) 2010-2018 "Alexander Kalkhof" [email:entc@kalkhof.org]
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

#include "sys/entc_types.h"
#include <types/ecbuffer.h>

//=============================================================================

#include <time.h>

#pragma pack(push, 16)
typedef struct {
  
  uint_t msec;
  uint_t sec;
  
} EcTime;
#pragma pack(pop)

//-----------------------------------------------------------------------------

__ENTC_LIBEX void ectime_utc_current_time (EcTime*);

__ENTC_LIBEX void ectime_from_ttime (EcTime*, const time_t* time);

//=============================================================================

#pragma pack(push, 16)
typedef struct {

  uint_t day;
  uint_t month;
  uint_t year;
  
  uint_t hour;
  uint_t minute;
  
  uint_t usec;
  uint_t msec;
  uint_t sec;

  int isdst;
  
} EcDate;
#pragma pack(pop)

//-----------------------------------------------------------------------------

__ENTC_LIBEX void ectime_utc_date (EcDate*);

__ENTC_LIBEX void ectime_local_date (EcDate*);

__ENTC_LIBEX void ectime_date_from_time (EcDate*, const EcTime*);

__ENTC_LIBEX void ectime_date_utc_to_localtime (EcDate*);

//=============================================================================

__ENTC_LIBEX EcString ectime_current_utc_datetime ();

//-----------------------------------------------------------------------------

__ENTC_LIBEX void ectime_fmt (EcBuffer buf, const EcDate*, const EcString format);

// Sun, 11 May 2018 17:05:40 GMT
__ENTC_LIBEX void ectime_toGmtString (EcBuffer buf, const EcDate*);

// %Y%m%dT%H%M%SZ
__ENTC_LIBEX void ectime_toISO8601 (EcBuffer buf, const EcDate*);

__ENTC_LIBEX void ectime_toAlphaNum (EcBuffer buf, const EcDate*);

// YYYY-MM-DD HH:NN:SS
__ENTC_LIBEX void ectime_toString (EcBuffer buf, const EcDate*);

// YYYY_MM_DD__HH_NN_SS__
__ENTC_LIBEX void ectime_toPrefix (EcBuffer buf, const EcDate*);

__ENTC_LIBEX void ectime_toPaddedTimestamp (EcBuffer buf, const EcDate*);

__ENTC_LIBEX void ectime_parseISO8601 (time_t* t, const char* stime);

__ENTC_LIBEX void ectime_fromString (EcDate*, const char* stime);

//=============================================================================

struct EcStopWatch_s; typedef struct EcStopWatch_s* EcStopWatch;

//-----------------------------------------------------------------------------

__ENTC_LIBEX EcStopWatch ecstopwatch_create (ulong_t timeout);

__ENTC_LIBEX void ecstopwatch_destroy (EcStopWatch*);

__ENTC_LIBEX void ecstopwatch_start (EcStopWatch);

__ENTC_LIBEX ulong_t ecstopwatch_stop (EcStopWatch);

__ENTC_LIBEX int ecstopwatch_timedOut (EcStopWatch);

__ENTC_LIBEX int ecstopwatch_timedOutRef (EcStopWatch, EcStopWatch ref);

__ENTC_LIBEX void ecstopwatch_destroy (EcStopWatch*);

__ENTC_LIBEX ulong_t ecstopwatch_timeout (EcStopWatch);

//-----------------------------------------------------------------------------

#endif
