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

#define LOGFILE_TYPE_NONE 0
#define LOGFILE_TYPE_PIPE 1
#define LOGFILE_TYPE_REG 2
#define LOGFILE_TYPE_ERROR 3

#include <system/macros.h>
#include <system/types.h>

#include <types/eclist.h>
#include <types/ecstring.h>

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

struct EcLogger_s; typedef struct EcLogger_s* EcLogger;

__CPP_EXTERN______________________________________________________________________________START

/**
 * \brief constructor for the logger object
 *
 * \return a logger object
 */
__LIB_EXPORT EcLogger eclogger_new (ubyte_t threadid);

/**
 * \brief destructor for the logger object
 */
__LIB_EXPORT void eclogger_delete(EcLogger*);

__LIB_EXPORT void eclogger_sec (EcLogger, EcSecLevel);

/**
 * \brief logs a message
 *
 * \param ptr the object
 * \param mode the level of the: see \a LOGMSG_...
 * \param module the module description in 4 characters
 * \param msg the message
 */
__LIB_EXPORT void eclogger_log (EcLogger, EcLogLevel, const char* module, const char* msg);

/*
 * \brief logs a formated message
 *
 * \param ptr the object
 * \param mode the level of the: see \a LOGMSG_...
 * \param module the module description in 4 characters
 * \param format the format for the message: see \a printf
 */
__LIB_EXPORT void eclogger_logformat (EcLogger, EcLogLevel, const char* module, const char* format, ...);

/**
 * \brief logs the last error with a formated message
 *
 * \param ptr the object
 * \param mode the level of the: see \a LOGMSG_...
 * \param module the module description in 4 characters
 * \param format the format for the message: see \a printf
 */
__LIB_EXPORT void eclogger_logerrno (EcLogger, EcLogLevel, const char* module, const char* format, ...);

/**
 * \brief logs the last error with a formated message
 *
 * \param ptr the object
 * \param mode the level of the: see \a LOGMSG_...
 * \param module the module description in 4 characters
 * \param error the error number 
 * \param format the format for the message: see \a printf
 */
__LIB_EXPORT void eclogger_logerr (EcLogger, EcLogLevel, const char* module, int error, const char* format, ...);

/**
 * \brief logs data with an hexdecimal output
 *
 * \param ptr the object
 * \param mode the level of the: see \a LOGMSG_...
 * \param module the module description in 4 characters
 * \param data buffer containing the data
 * \param data size of the buffer 
 */
__LIB_EXPORT void eclogger_logbinary (EcLogger, EcLogLevel, const char* module, const char* data, uint_t size);
  
#ifndef __DOS__

__LIB_EXPORT void eclogger_paramLogFile(EcLogger, const EcString confdir, int argc, char *argv[]);
  
__LIB_EXPORT void eclogger_setLogFile(EcLogger, const EcString logfile, const EcString confdir);
  
__LIB_EXPORT const char* eclogger_getLogFile(EcLogger);
  
__LIB_EXPORT EcList eclogger_errmsgs(EcLogger);

__LIB_EXPORT EcList eclogger_sccmsgs(EcLogger);
  
__LIB_EXPORT uint_t eclogger_securityIncidents(EcLogger);
  
__LIB_EXPORT void eclogger_clear(EcLogger);

#endif

__CPP_EXTERN______________________________________________________________________________END

#endif
