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

#ifndef ADBO_SCHEMA_H
#define ADBO_SCHEMA_H 1

#include <system/macros.h>

#include "adbo_types.h"

__CPP_EXTERN______________________________________________________________________________START

__LIB_EXPORT AdboSchema adbo_schema_new (AdboContext, const EcString dbsource);

__LIB_EXPORT void adbo_schema_del (AdboSchema*);

__LIB_EXPORT AdboObject adbo_schema_get (AdboSchema, AdboContext, AdboContainer parent, const EcString tablename, const EcString origin, AdboValue value);

__LIB_EXPORT void adbo_schema_ref (AdboSchema, AdboContext, AdboContainer parent, const EcString tablename, EcList objects, const EcString origin);

__CPP_EXTERN______________________________________________________________________________END

#endif
