/*
 * Copyright (c) 2010-2013 "Alexander Kalkhof" [email:adbo@kalkhof.org]
 *
 * This file is part of adbo framework (Advanced Database Objects)
 *
 * adbo is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * adbo is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with adbo. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef ADBO_VALUE_H
#define ADBO_VALUE_H 1

#include <system/macros.h>

#include "adbo_types.h"

struct AdboValue_s; typedef struct AdboValue_s* AdboValue;

__CPP_EXTERN______________________________________________________________________________START

// constructor
__LIB_EXPORT AdboValue adbo_value_new (const EcString dbcolumn, const EcString data, const EcString link);

__LIB_EXPORT AdboValue adbo_value_newFromXml (EcXMLStream);

__LIB_EXPORT void adbo_value_del (AdboValue*);

__LIB_EXPORT AdboValue adbo_value_clone (const AdboValue);

__LIB_EXPORT void adbo_value_set (AdboValue, const EcString, int state);

__LIB_EXPORT AdboObject adbo_value_cobject (AdboValue, AdboContainer);

__LIB_EXPORT AdboValue adbo_value_cseek (AdboValue, AdboContainer);

__LIB_EXPORT const EcString adbo_value_cget (AdboValue, AdboContainer);

__LIB_EXPORT const EcString adbo_value_get (AdboValue, AdboObject);

__LIB_EXPORT AdboValue adbo_value_seek (AdboValue, AdboObject);

__LIB_EXPORT const EcString adbo_value_getDBColumn (AdboValue);

__LIB_EXPORT const EcString adbo_value_getData (AdboValue);

__LIB_EXPORT int adbo_value_getState (AdboValue);

__LIB_EXPORT int adbo_value_hasLocalLink (AdboValue);

__LIB_EXPORT void adbo_value_commit (AdboValue);

__LIB_EXPORT void adbo_value_rollback (AdboValue);

__CPP_EXTERN______________________________________________________________________________END

#endif
