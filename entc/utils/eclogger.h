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

#ifndef ENTC_UTILS_LOGGER_H
#define ENTC_UTILS_LOGGER_H 1

#include <system/macros.h>
#include <system/types.h>

#include <types/eclist.h>
#include <types/ecstring.h>
#include <types/ecudc.h>

typedef enum
{
  LL_FATAL   = 1,
  LL_ERROR   = 2,
  LL_WARN    = 3,
  LL_INFO    = 4,
  LL_DEBUG   = 5,
  LL_TRACE   = 6
} EcLogLevel;

typedef enum
{
  
  SL_RED     = 1,
  SL_YELLOW  = 2
  
} EcSecLevel;

typedef void (*eclogger_log_fct)      (void* ptr, EcLogLevel, ubyte_t, const EcString module, const EcString msg);
typedef EcUdc (*eclogger_message_fct)  (void* ptr, uint_t logid, uint_t messageid, EcUdc* data);

typedef struct {
  
  eclogger_log_fct logfct;
  eclogger_message_fct msgfct;
  void* ptr;
  
} EcLoggerCallbacks;

struct EcLogger_s; typedef struct EcLogger_s* EcLogger;

struct EcEchoLogger_s; typedef struct EcEchoLogger_s* EcEchoLogger;
struct EcFileLogger_s; typedef struct EcFileLogger_s* EcFileLogger;
struct EcListLogger_s; typedef struct EcListLogger_s* EcListLogger;

__CPP_EXTERN______________________________________________________________________________START

// constructor for the logger object (creates a console logger per default)
__LIB_EXPORT EcLogger eclogger_new (ubyte_t threadid);

// destructor for the logger object
__LIB_EXPORT void eclogger_del (EcLogger*);

// set the callback (callback + object)
__LIB_EXPORT void eclogger_setCallback (EcLogger, EcLoggerCallbacks*);

__LIB_EXPORT void eclogger_getCallback (EcLogger, EcLoggerCallbacks*); 

// set the callback from existing logger
__LIB_EXPORT void eclogger_sync (EcLogger, const EcLogger);

// set log level
__LIB_EXPORT void eclogger_setLogLevel (EcLogger, EcLogLevel);

// ----- logger methods -----------------------------------------------------------------------

__LIB_EXPORT void eclogger_log (EcLogger, EcLogLevel, const char* module, const char* msg);

__LIB_EXPORT void eclogger_logformat (EcLogger, EcLogLevel, const char* module, const char* format, ...);

__LIB_EXPORT void eclogger_logerrno (EcLogger, EcLogLevel, const char* module, const char* format, ...);

__LIB_EXPORT void eclogger_logerr (EcLogger, EcLogLevel, const char* module, int error, const char* format, ...);

__LIB_EXPORT void eclogger_logbinary (EcLogger, EcLogLevel, const char* module, const char* data, uint_t size);

// ----- message methods ----------------------------------------------------------------------

__LIB_EXPORT EcUdc eclogger_message (EcLogger, uint_t logid, uint_t messageid, EcUdc* data);

// ----- security methods ---------------------------------------------------------------------

__LIB_EXPORT void eclogger_sec (EcLogger, EcSecLevel);

__LIB_EXPORT uint_t eclogger_securityIncidents (EcLogger);

// ----- data methods -------------------------------------------------------------------------

__LIB_EXPORT EcList eclogger_errmsgs (EcLogger);

__LIB_EXPORT EcList eclogger_sccmsgs (EcLogger);

__LIB_EXPORT void eclogger_clear (EcLogger);

// ***** console logger ***********************************************************************

// constructor for the logger object (creates a console logger per default)
__LIB_EXPORT EcEchoLogger ecechologger_new ();

// destructor for the logger object
__LIB_EXPORT void ecechologger_del (EcEchoLogger*);

// get the callback
__LIB_EXPORT void ecechologger_getCallback (EcEchoLogger, EcLoggerCallbacks*);

// ***** file logger **************************************************************************

// constructor for the logger object (creates a console logger per default)
__LIB_EXPORT EcFileLogger ecfilelogger_new (const EcString filename);

// destructor for the logger object
__LIB_EXPORT void ecfilelogger_del (EcFileLogger*);

// get the callback
__LIB_EXPORT void ecfilelogger_getCallback (EcFileLogger, EcLoggerCallbacks*);

// ***** list logger **************************************************************************

// constructor for the logger object (creates a console logger per default)
__LIB_EXPORT EcListLogger eclistlogger_new ();

// destructor for the logger object
__LIB_EXPORT void eclistlogger_del (EcListLogger*);

// get the callback
__LIB_EXPORT void eclistlogger_getCallback (EcListLogger, EcLoggerCallbacks*);

__LIB_EXPORT void eclistlogger_register (EcListLogger, EcLogger);

__LIB_EXPORT EcListNode eclistlogger_add (EcListLogger, const EcLoggerCallbacks*);

__LIB_EXPORT void eclistlogger_remove (EcListLogger, EcListNode);

__CPP_EXTERN______________________________________________________________________________END

#endif
