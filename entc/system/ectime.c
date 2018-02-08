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

void ectime_getTime (EcTime* ectime)
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

void ectime_toString (const time_t* t, char* buffer, ulong_t size)
{
  struct tm timeinfo;

  // fill the timeinfo
  ectime_toTimeInfo (&timeinfo, t);

  // create buffer with timeinfo as string
  strftime (buffer, size, "%Y-%m-%d %H:%M:%S", &timeinfo);  
}

//-----------------------------------------------------------------------------------

void ectime_toPrefix (const time_t* t, char* buffer, ulong_t size)
{
  struct tm timeinfo;
  
  // fill the timeinfo
  ectime_toTimeInfo (&timeinfo, t);
  
  // create buffer with timeinfo as string
  strftime (buffer, size, "%Y_%m_%d__%H_%M_%S__", &timeinfo);
}

//-----------------------------------------------------------------------------------

void ectime_toAlphaNum (const time_t* t, char* buffer, ulong_t size)
{
  struct tm timeinfo;
  
  // fill the timeinfo
  ectime_toTimeInfo (&timeinfo, t);

  // create buffer with timeinfo as string
  strftime (buffer, size, "%a, %e %b %Y %H:%M:%S %z", &timeinfo);
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

void ectime_toPaddedTimestamp (time_t* t, char* buffer, ulong_t size)
{
  EcString h;

#ifdef _WIN32

  _snprintf_s(buffer, size, size, "%li", *t); 

#else
  struct tm timeinfo;
  
  // fill the timeinfo
  ectime_toTimeInfo (&timeinfo, t);

  // create buffer with timeinfo as string
  strftime (buffer, size, "%s", &timeinfo);

#endif

  h = ecstr_lpad (buffer, '0', size);
  
  memcpy (buffer, h, size);
  buffer [size] = 0;
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
