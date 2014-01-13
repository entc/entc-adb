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

#ifndef ADBO_OBJECT_H
#define ADBO_OBJECT_H 1

#include <system/macros.h>
#include <system/types.h>
#include <types/eclist.h>

#include "adbo_types.h"
#include "adbo_value.h"

__CPP_EXTERN______________________________________________________________________________START

// constructor (creates a new empty object from xml config)
__LIB_EXPORT AdboObject adbo_object_new1 (AdboContainer parent, AdboContext, uint_t type, EcXMLStream, const EcString);

// constructor (creates a new empty object from xml config)
__LIB_EXPORT AdboObject adbo_object_new2 (AdboContainer parent, AdboContext, uint_t type, AdblTable* table_info, const EcString origin, AdboValue value);

// destructor (deletes recursively object and data)
__LIB_EXPORT void adbo_object_del (AdboObject*);

// clone (clones the object recursively, but no data)
__LIB_EXPORT AdboObject adbo_object_clone (const AdboObject, AdboContainer parent);

// request (fill structure recursively with new data from database backend, overrides existing data)
__LIB_EXPORT int adbo_object_request (AdboObject, AdboContext);

// update (write data back recursively to database backend, only works with filled object)
__LIB_EXPORT int adbo_object_update (AdboObject, AdboContext, int withTransaction);

// delete (removes this node from database)
__LIB_EXPORT int adbo_object_delete (AdboObject, AdboContext, int withTransaction);

// misc

__LIB_EXPORT void adbo_objects_fromXml (AdboContainer, AdboContext, EcXMLStream, const EcString tag);

__LIB_EXPORT AdboValue adbo_getValue (AdboObject);

__LIB_EXPORT void adbo_setValue (AdboObject, AdboValue);

__LIB_EXPORT void adbo_strToStream (AdboObject, EcStream);

__LIB_EXPORT void adbo_object_transaction (AdboObject, int state);

__LIB_EXPORT void adbo_object_addToQuery (AdboObject, AdblQuery*);

__LIB_EXPORT void adbo_object_setFromQuery (AdboObject, AdblCursor*, EcLogger);

__LIB_EXPORT void adbo_object_addToAttr (AdboObject, AdboContainer, AdblAttributes*);

// accessors

__LIB_EXPORT AdboObject adbo_at (AdboObject, const EcString link);

__LIB_EXPORT EcString adbo_str (AdboObject);

__LIB_EXPORT int adbo_set (AdboObject, const EcString);

__LIB_EXPORT int adbo_add (AdboObject);

__LIB_EXPORT AdboObject adbo_get (AdboObject, const EcString);

// debug

__LIB_EXPORT void adbo_dump (AdboObject, EcLogger logger);

__LIB_EXPORT void adbo_dump_next (AdboObject, AdboContainer, int depth, int le, EcBuffer b2, EcLogger logger);

__LIB_EXPORT const EcString adbo_dump_state (uint_t state);

__CPP_EXTERN______________________________________________________________________________END

#endif
