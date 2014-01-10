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

#ifndef ADBO_CONTAINER_H
#define ADBO_CONTAINER_H 1

#define ADBO_CONTAINER_NODE         1
#define ADBO_CONTAINER_ITEM         2
#define ADBO_CONTAINER_SUBSTITUTE   3

#include <system/macros.h>

#include "adbo_types.h"

typedef void (*container_callback_fct)(void* ptr1, void* ptr2, void* data1, void* data2);

typedef void (*iterator_callback_fct)(void* ptr, void* data);

__CPP_EXTERN______________________________________________________________________________START

// constructor
__LIB_EXPORT AdboContainer adbo_container_new (uint_t type, AdboContainer parent);

// destructor
__LIB_EXPORT void adbo_container_del (AdboContainer*);

// copy constructor
__LIB_EXPORT AdboContainer adbo_container_clone (const AdboContainer oself, AdboContainer parent, container_callback_fct, void* ptr1, void* ptr2);

// set all values from db
__LIB_EXPORT void adbo_container_set (AdboContainer, AdblCursor*, EcLogger);

// construct query from container
__LIB_EXPORT void adbo_container_query (AdboContainer, AdblQuery*);

// construct attributes from container
__LIB_EXPORT void adbo_container_attrs (AdboContainer, AdblAttributes* attrs);

// run the request on the container
__LIB_EXPORT int adbo_container_request (AdboContainer, AdboContext context);

// run the update on the container
__LIB_EXPORT int adbo_container_update (AdboContainer, AdboContext context);

// run the delete on the container
__LIB_EXPORT int adbo_container_delete (AdboContainer, AdboContext context);

// set transaction state
__LIB_EXPORT void adbo_container_transaction (AdboContainer, int state);

// convert content into string stream
__LIB_EXPORT void adbo_container_str (AdboContainer, EcStream stream);

// get the object only from forward links
__LIB_EXPORT AdboObject adbo_container_get (AdboContainer, const EcString link);

// get the object related to the link
__LIB_EXPORT AdboObject adbo_container_at (AdboContainer, const EcString link);

// add / assign object to container
__LIB_EXPORT void adbo_container_add (AdboContainer, AdboObject);

// iterate through all elements
__LIB_EXPORT void adbo_container_iterate (AdboContainer, iterator_callback_fct, void*);

// return parent
__LIB_EXPORT AdboContainer adbo_container_parent (AdboContainer);

// dump all elements
__LIB_EXPORT void adbo_container_dump (AdboContainer, int tab, EcBuffer, EcLogger);

__CPP_EXTERN______________________________________________________________________________END

#endif
