#include "ectime.h"

#include "types/ecstring.h"
#include "tools/eclog.h"

#include <errno.h>
#include <stdio.h>

#if defined _WIN64 || defined _WIN32

#include <windows.h>

#elif __APPLE_CC__

#include <sys/time.h>

#else

#include <time.h>
#include <sys/time.h>

#endif

//-----------------------------------------------------------------------------------

void ectime_utc_current_time (EcTime* ectime)
{

#if defined _WIN64 || defined _WIN32

  SYSTEMTIME time;
  GetSystemTime(&time);
  
  ectime->sec = time.wSecond;
  ectime->msec = time.wMilliseconds;

#else

  struct timeval time;  
  gettimeofday (&time, NULL);
  
  ectime->sec = time.tv_sec;
  ectime->msec = time.tv_usec;
  
#endif
  
}

//-----------------------------------------------------------------------------------

void ectime_from_ttime (EcTime* ectime, const time_t* time)
{
  ectime->sec = *time;  // we have only precision in seconds
  ectime->msec = 0;
}

//-----------------------------------------------------------------------------------

void ectime_convert_ecdate_to_timeinfo (struct tm* timeinfo, const EcDate* ecdate)
{
  // fill the timeinfo
  timeinfo->tm_sec   = ecdate->sec;
  timeinfo->tm_hour  = ecdate->hour;
  timeinfo->tm_min   = ecdate->minute;
  
  timeinfo->tm_mday  = ecdate->day;
  timeinfo->tm_mon   = ecdate->month - 1;
  timeinfo->tm_year  = ecdate->year - 1900;  
  
  timeinfo->tm_isdst = ecdate->isdst;
}

//-----------------------------------------------------------------------------------

void ectime_convert_timeinfo_to_ecdate (EcDate* ecdate, const struct tm* timeinfo)
{
  // fill the timeinfo
  ecdate->sec = timeinfo->tm_sec;
  ecdate->msec = 0;
  ecdate->usec = 0;

  ecdate->minute = timeinfo->tm_min;
  ecdate->hour = timeinfo->tm_hour;
  
  ecdate->isdst = timeinfo->tm_isdst;
  
  ecdate->day = timeinfo->tm_mday;
  ecdate->month = timeinfo->tm_mon + 1;
  ecdate->year = timeinfo->tm_year + 1900; 
}

//-----------------------------------------------------------------------------------

void ectime_date_from_time (EcDate* ecdate, const EcTime* ectime)
{
  time_t t = ectime->sec;
  struct tm* l01 = gmtime (&t);
  
  ectime_convert_timeinfo_to_ecdate (ecdate, l01); 
}

//-----------------------------------------------------------------------------------

void ectime_utc_date (EcDate* ecdate)
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
  l01 = gmtime (&(time.tv_sec));
  
  ectime_convert_timeinfo_to_ecdate (ecdate, l01); 
 
  ecdate->msec = time.tv_usec / 1000;
  ecdate->usec = time.tv_usec;
  
#endif
}

//-----------------------------------------------------------------------------------

void ectime_local_date (EcDate* ecdate)
{

#if defined _WIN64 || defined _WIN32
  
  SYSTEMTIME time;
  GetLocalTime(&time);
  
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
  
  ectime_convert_timeinfo_to_ecdate (ecdate, l01);
  
  ecdate->msec = time.tv_usec / 1000;
  ecdate->usec = time.tv_usec;  
  
#endif
  
}

//-----------------------------------------------------------------------------------

void ectime_date_utc_to_localtime (EcDate* ecdate)
{
#if defined _WIN64 || defined _WIN32

#else

  struct tm timeinfo;
  struct tm* l01;
  time_t t_of_day;
  uint_t msec;
  
  ectime_convert_ecdate_to_timeinfo (&timeinfo, ecdate);
  
  // turn on automated DST, depends on the timezone
  timeinfo.tm_isdst = -1;
  
  // the function takes a local time and calculate extra values
  // in our case it is UTC, so we will get the (localtime) UTC value
  // because the function also applies timezone conversion
  t_of_day = mktime (&timeinfo);
  
  // correct (localtime) UTC to (current) UTC :-)
  t_of_day = t_of_day + timeinfo.tm_gmtoff;
  
  // now get the localtime from our UTC
  l01 = localtime (&t_of_day);
  
  // save the msec
  msec = ecdate->msec;
  
  ectime_convert_timeinfo_to_ecdate (ecdate, l01);

  ecdate->msec = msec;
  
#endif
}

//-----------------------------------------------------------------------------------

void ectime_fmt (EcBuffer buf, const EcDate* ecdate, const EcString format)
{
  struct tm timeinfo;
  
  ectime_convert_ecdate_to_timeinfo (&timeinfo, ecdate);
  
  mktime (&timeinfo);
  
  // create buffer with timeinfo as string
  strftime ((char*)buf->buffer, buf->size, format, &timeinfo);
}

//-----------------------------------------------------------------------------------

void ectime_toGmtString (EcBuffer buf, const EcDate* ecdate)
{
  ectime_fmt (buf, ecdate, "%a, %d %b %Y %H:%M:%S GMT");
}

//-----------------------------------------------------------------------------------

void ectime_toISO8601 (EcBuffer buf, const EcDate* ecdate)
{
  ectime_fmt (buf, ecdate, "%Y%m%dT%H%M%SZ");
}

//-----------------------------------------------------------------------------------

void ectime_toString (EcBuffer buf, const EcDate* ecdate)
{
  // resize to fit the date
  ecbuf_resize (buf, 21);
  
  // format
  ecbuf_format (buf, 20, "%i-%02i-%02i %02i:%02i:%02i", ecdate->year, ecdate->month, ecdate->day, ecdate->hour, ecdate->minute, ecdate->sec);
}

//-----------------------------------------------------------------------------------

void ectime_toPrefix (EcBuffer buf, const EcDate* ecdate)
{
  ectime_fmt (buf, ecdate, "%Y_%m_%d__%H_%M_%S__");
}

//-----------------------------------------------------------------------------------

void ectime_toAlphaNum (EcBuffer buf, const EcDate* ecdate)
{
  ectime_fmt (buf, ecdate, "%a, %e %b %Y %H:%M:%S %z");
}

//-----------------------------------------------------------------------------------

void ectime_toPaddedTimestamp (EcBuffer buf, const EcDate* ecdate)
{
  EcString h;

#ifdef _WIN32

  time_t t;
  struct tm timeinfo;
  
  // fill the timeinfo
  ectime_convert_ecdate_to_timeinfo (&timeinfo, ecdate);

  t = mktime (&timeinfo);

  _snprintf_s(buf->buffer, buf->size, buf->size, "%li", t); 

#else
  struct tm timeinfo;
  
  // fill the timeinfo
  ectime_convert_ecdate_to_timeinfo (&timeinfo, ecdate);

  // create buffer with timeinfo as string
  strftime ((char*)buf->buffer, buf->size, "%s", &timeinfo);

#endif

  h = ecstr_lpad ((char*)buf->buffer, '0', buf->size);
  
  memcpy (buf->buffer, h, buf->size);
  buf->buffer [buf->size] = 0;
}

//-----------------------------------------------------------------------------------

void ectime_parseISO8601 (time_t* t, const char* stime)
{
  struct tm timeinfo;  
  int year, month, day, hour, minute, second;

#ifdef _WIN32
  int res = sscanf_s (stime, "%4d%2d%2dT%2d%2d%2d", &year, &month, &day, &hour, &minute, &second);
#else
  int res = sscanf (stime, "%4d%2d%2dT%2d%2d%2d", &year, &month, &day, &hour, &minute, &second);
#endif

  if (res != 6)
  {
    eclog_fmt (LL_ERROR, "ENTC", "parse iso8601", "can't parse time string '%s', %i", stime, res);
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

void ectime_fromString (EcDate* ed, const char* stime)
{
#ifdef _WIN32
  int res = sscanf_s (stime, "%4d-%2d-%2d %2d:%2d:%2d", &(ed->year), &(ed->month), &(ed->day), &(ed->hour), &(ed->minute), &(ed->sec));
#else
  int res = sscanf (stime, "%4d-%2d-%2d %2d:%2d:%2d", &(ed->year), &(ed->month), &(ed->day), &(ed->hour), &(ed->minute), &(ed->sec));
#endif

  if (res != 6)
  {
    eclog_fmt (LL_ERROR, "ENTC", "parse iso8601", "can't parse time string '%s', %i", stime, res);
  }  
}

//-----------------------------------------------------------------------------------

EcString ectime_current_utc_datetime ()
{
  EcBuffer buf = ecbuf_create (21);
  
  // fetch current utc date
  EcDate d1;
  ectime_utc_date (&d1);
  
  ectime_toString (buf, &d1);
  
  return ecbuf_str (&buf);
}

//-----------------------------------------------------------------------------------

struct EcStopWatch_s
{
#ifdef _WIN32
  DWORD start;
#else
  struct timeval start;
#endif  

  ulong_t timeout;
  
};

//-----------------------------------------------------------------------------------

EcStopWatch ecstopwatch_create (ulong_t timeout)
{
  EcStopWatch self = ENTC_NEW (struct EcStopWatch_s);
  
  self->timeout = timeout;
  
  return self;
}

//-----------------------------------------------------------------------------------

void ecstopwatch_destroy (EcStopWatch* pself)
{
  ENTC_DEL (pself, struct EcStopWatch_s);
}

//-----------------------------------------------------------------------------------

void ecstopwatch_start (EcStopWatch self)
{
#ifdef _WIN32
  self->start = GetTickCount();
#else
  gettimeofday (&(self->start), NULL);
#endif
}

//-----------------------------------------------------------------------------------

ulong_t ecstopwatch_stop (EcStopWatch self)
{
#ifdef _WIN32
  DWORD end = GetTickCount();

  return end - self->start;
#else
  struct timeval end;
  double elapsedTime;

  gettimeofday (&end, NULL);

  elapsedTime  = (end.tv_sec - self->start.tv_sec) * 1000.0;      // sec to ms
  elapsedTime += (end.tv_usec - self->start.tv_usec) / 1000.0;    // us to ms
  
  return elapsedTime;
#endif
}

//-----------------------------------------------------------------------------------

int ecstopwatch_timedOut (EcStopWatch self)
{
  if (self->timeout == ENTC_INFINITE)
  {
    return FALSE;
  }

  return ecstopwatch_stop (self) > self->timeout;
}

//-----------------------------------------------------------------------------------

int ecstopwatch_timedOutRef (EcStopWatch self, EcStopWatch ref)
{
#ifdef _WIN32
  DWORD end = GetTickCount();

  return (end - self->start) > self->timeout;
#else
  double elapsedTime;

  if (self->timeout == ENTC_INFINITE)
  {
    return FALSE;
  }
  
  elapsedTime  = (ref->start.tv_sec - self->start.tv_sec) * 1000.0;      // sec to ms
  elapsedTime += (ref->start.tv_usec - self->start.tv_usec) / 1000.0;    // us to ms
  
  return elapsedTime > self->timeout;
#endif
}

//-----------------------------------------------------------------------------------

ulong_t ecstopwatch_timeout (EcStopWatch self)
{
  return self->timeout;
} 

//-----------------------------------------------------------------------------------
