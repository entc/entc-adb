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

#include <sys/entc_export.h>
#include "adbo_types.h"

typedef void (*container_callback_fct)(void* ptr1, void* ptr2, void* data1, void* data2);

typedef void (*iterator_callback_fct)(void* ptr, void* data);

// constructor
__ENTC_LIBEX AdboContainer adbo_container_new (uint_t type, AdboContainer parent);

// destructor
__ENTC_LIBEX void adbo_container_del (AdboContainer*);

// copy constructor
__ENTC_LIBEX AdboContainer adbo_container_clone (const AdboContainer oself, AdboContainer parent, container_callback_fct, void* ptr1, void* ptr2);

// set all values from db
//__ENTC_LIBEX void adbo_container_set (AdboContainer, AdblCursor*, EcLogger);

// construct query from container
//__ENTC_LIBEX void adbo_container_query (AdboContainer, AdblQuery*);

// construct attributes from container
//__ENTC_LIBEX void adbo_container_attrs (AdboContainer, AdblAttributes* attrs);

// run the request on the container
__ENTC_LIBEX int adbo_container_request (AdboContainer, AdboContext context, EcUdc conditions, int depth, int dpos);

// run the update on the container
__ENTC_LIBEX int adbo_container_update (AdboContainer, AdboContext context);

// run the delete on the container
__ENTC_LIBEX int adbo_container_delete (AdboContainer, AdboContext context);

// set transaction state
__ENTC_LIBEX void adbo_container_transaction (AdboContainer, int state);

// convert content into string stream
__ENTC_LIBEX void adbo_container_str (AdboContainer, EcStream stream);

// get the object only from forward links
__ENTC_LIBEX AdboObject adbo_container_get (AdboContainer, const EcString link);

// get the object related to the link
__ENTC_LIBEX AdboObject adbo_container_at (AdboContainer, const EcString link);

// add / assign object to container
__ENTC_LIBEX void adbo_container_add (AdboContainer, AdboObject);

// iterate through all elements
__ENTC_LIBEX void adbo_container_iterate (AdboContainer, iterator_callback_fct, void*);

// return parent
__ENTC_LIBEX AdboContainer adbo_container_parent (AdboContainer);

// fill udc with content
__ENTC_LIBEX EcUdc adbo_container_udc (AdboContainer);

// dump all elements
__ENTC_LIBEX void adbo_container_dump (AdboContainer, int tab, EcBuffer);

#endif
