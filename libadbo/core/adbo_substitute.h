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

#ifndef ADBO_SUBSTITUTE_H
#define ADBO_SUBSTITUTE_H 1

#include <system/macros.h>
#include "adbo_types.h"

struct AdboSubstitute_s; typedef struct AdboSubstitute_s* AdboSubstitute;

__CPP_EXTERN______________________________________________________________________________START

// constructor
__LIB_EXPORT AdboSubManager adbo_subsmgr_new (const EcString scanpath, AdboContext);

// destructor
__LIB_EXPORT void adbo_subsmgr_del (AdboSubManager*);

// getter
__LIB_EXPORT AdboObject adbo_subsmgr_get (AdboSubManager, const EcString name);

// constructor
__LIB_EXPORT AdboSubstitute adbo_substitute_new (AdboObject, AdboContext, AdboContainer, EcXMLStream);

// destructor
__LIB_EXPORT void adbo_substitute_del (AdboSubstitute*);

// constructor
__LIB_EXPORT AdboSubstitute adbo_substitute_clone (const AdboSubstitute, AdboContainer parent);

// fill data from database backend
__LIB_EXPORT int adbo_substitute_request (AdboSubstitute, AdboContext, int depth, int dpos);

// save data to database backend
__LIB_EXPORT int adbo_substitute_update (AdboSubstitute, AdboContext, int withTransaction);

// save data to database backend
__LIB_EXPORT int adbo_substitute_delete (AdboSubstitute, AdboContext, int withTransaction);

// apply transaction state
__LIB_EXPORT void adbo_substitute_transaction (AdboSubstitute, int state);

__LIB_EXPORT AdboObject adbo_substitute_at (AdboSubstitute, const EcString);

__LIB_EXPORT void adbo_substitute_strToStream (AdboSubstitute, EcStream);

__LIB_EXPORT EcString adbo_substitute_str (AdboSubstitute);

__LIB_EXPORT AdboObject adbo_substitute_get (AdboSubstitute, const EcString link);

__LIB_EXPORT void adbo_substitute_dump (AdboObject, AdboSubstitute, int tab, int le, EcBuffer, EcLogger);

__LIB_EXPORT void adbo_substitute_addToQuery (AdboSubstitute, AdblQuery*);

__LIB_EXPORT void adbo_substitute_setFromQuery (AdboSubstitute, AdblCursor*, EcLogger);

__LIB_EXPORT void adbo_substitute_addToAttr (AdboSubstitute, AdblAttributes*);

__CPP_EXTERN______________________________________________________________________________END

#endif
