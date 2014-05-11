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

#ifndef ADBO_NODE_H
#define ADBO_NODE_H 1

#include <system/macros.h>

#include "adbo_types.h"

struct AdboNode_s; typedef struct AdboNode_s* AdboNode;

__CPP_EXTERN______________________________________________________________________________START



// constructor
__LIB_EXPORT AdboNode adbo_node_new1 (AdboObject, AdboContext, AdboContainer, EcXMLStream);

//__LIB_EXPORT AdboNode adbo_node_new2 (AdboObject, AdboContext, AdboContainer, AdblTable* table_info, const EcString origin);

// destructor
__LIB_EXPORT void adbo_node_del (AdboNode*);

// constructor
__LIB_EXPORT AdboNode adbo_node_clone (const AdboNode, AdboContainer parent);

// fill data from database backend
__LIB_EXPORT int adbo_node_request (AdboNode, AdboContext, EcUdc, int depth, int dpos);

// save data to database backend
__LIB_EXPORT int adbo_node_delete (AdboNode, AdboContext, int withTransaction);

// apply transaction state
__LIB_EXPORT void adbo_node_transaction (AdboNode, int state);

__LIB_EXPORT AdboObject adbo_node_at (AdboNode, const EcString);

__LIB_EXPORT void adbo_node_strToStream (AdboNode, EcStream);

__LIB_EXPORT EcString adbo_node_str (AdboNode);

__LIB_EXPORT AdboObject adbo_node_get (AdboObject, AdboNode, const EcString link);

__LIB_EXPORT EcUdc adbo_node_udc (AdboObject, AdboNode);

__LIB_EXPORT void adbo_node_clear (AdboNode);

__LIB_EXPORT void adbo_node_dump (AdboObject, AdboNode, int tab, int le, EcBuffer, EcLogger);

__CPP_EXTERN______________________________________________________________________________END

#endif
