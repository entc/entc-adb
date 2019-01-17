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

#ifndef ENTC_TOOLS_LOG_H
#define ENTC_TOOLS_LOG_H 1

#include "sys/entc_export.h"
#include "sys/entc_types.h"

//-----------------------------------------------------------------------------

typedef enum
{
  LL_FATAL   = 1,
  LL_ERROR   = 2,
  LL_WARN    = 3,
  LL_INFO    = 4,
  LL_DEBUG   = 5,
  LL_TRACE   = 6

} EcLogLevel;

//-----------------------------------------------------------------------------

__ENTC_LIBEX void eclog_msg (EcLogLevel, const char* unit, const char* method, const char* msg);

__ENTC_LIBEX void eclog_fmt (EcLogLevel, const char* unit, const char* method, const char* format, ...);

__ENTC_LIBEX void eclog_bin (EcLogLevel, const char* unit, const char* method, const char* data, uint64_t size);

__ENTC_LIBEX void eclog_err_os (EcLogLevel, const char* unit, const char* method, const char* format, ...);

//-----------------------------------------------------------------------------

#endif
