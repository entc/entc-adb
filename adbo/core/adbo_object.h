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

#include <sys/entc_export.h>
#include <types/eclist.h>
#include <types/ecudc.h>

#include "adbo_types.h"
#include "adbo_value.h"

// constructor (creates a new empty object from xml config)
__ENTC_LIBEX AdboObject adbo_object_new1 (AdboContainer parent, AdboContext, uint_t type, EcXMLStream, const EcString);

// constructor (creates a new empty object from database structure)
//__ENTC_LIBEX AdboObject adbo_object_new2 (AdboContainer parent, AdboContext, uint_t type, AdblTable* table_info, const EcString origin, AdboValue value);

// destructor (deletes recursively object and data)
__ENTC_LIBEX void adbo_object_del (AdboObject*);

// clone (clones the object recursively, but no data)
__ENTC_LIBEX AdboObject adbo_object_clone (const AdboObject, AdboContainer parent);

// request (fill structure recursively with new data from database backend, overrides existing data)
// depth defines the recursive depth, default should be 1, -1 is infinite
// dpos is always 0 rom beginning
__ENTC_LIBEX int adbo_object_request (AdboObject, AdboContext, EcUdc, int depth, int dpos);

// update (write data back recursively to database backend, only works with filled object)
__ENTC_LIBEX int adbo_object_update (AdboObject, AdboContext, int withTransaction);

// delete (removes this node from database)
__ENTC_LIBEX int adbo_object_delete (AdboObject, AdboContext, int withTransaction);

// misc

__ENTC_LIBEX void adbo_objects_fromXml (AdboContainer, AdboContext, EcXMLStream, const EcString tag);

__ENTC_LIBEX AdboValue adbo_getValue (AdboObject);

__ENTC_LIBEX void adbo_setValue (AdboObject, AdboValue);

__ENTC_LIBEX void adbo_strToStream (AdboObject, EcStream);

__ENTC_LIBEX void adbo_object_transaction (AdboObject, int state);

//__ENTC_LIBEX void adbo_object_addToQuery (AdboObject, AdblQuery*);

//__ENTC_LIBEX void adbo_object_setFromQuery (AdboObject, AdblCursor*, EcLogger);

//__ENTC_LIBEX void adbo_object_addToAttr (AdboObject, AdboContainer, AdblAttributes*);

// accessors

__ENTC_LIBEX AdboObject adbo_at (AdboObject, const EcString link);

__ENTC_LIBEX EcString adbo_str (AdboObject);

__ENTC_LIBEX int adbo_set (AdboObject, const EcString);

__ENTC_LIBEX int adbo_add (AdboObject);

__ENTC_LIBEX AdboObject adbo_get (AdboObject, const EcString);

__ENTC_LIBEX EcUdc adbo_udc (AdboObject);

// debug

__ENTC_LIBEX void adbo_dump (AdboObject);

__ENTC_LIBEX void adbo_dump_next (AdboObject, AdboContainer, int depth, int le, EcBuffer b2);

__ENTC_LIBEX const EcString adbo_dump_state (uint_t state);

#endif
