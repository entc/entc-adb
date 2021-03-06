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

#ifndef ENTC_TOOLS_XMLDOM_H
#define ENTC_TOOLS_XMLDOM_H 1

#include "../system/macros.h"
#include "../system/types.h"

#include "../types/ecstring.h"
#include "../types/ecbuffer.h"
#include "../types/ecudc.h"

__CPP_EXTERN______________________________________________________________________________START

__LIB_EXPORT EcUdc ecxmldom_create_tag (EcUdc parent, const EcString name);

__LIB_EXPORT const EcString ecxmldom_get_name (EcUdc tag);

__LIB_EXPORT void ecxmldom_set_name (EcUdc, const EcString name);

__LIB_EXPORT void ecxmldom_set_value (EcUdc, const EcString value);

__LIB_EXPORT void ecxmldom_add_attribute (EcUdc tag, const EcString name, const EcString value);

__LIB_EXPORT EcBuffer ecxmldom_buffer (EcUdc*);

__LIB_EXPORT void ecxmldom_setNamespace (EcUdc, const EcString, const EcString definition);

__LIB_EXPORT void ecxmldom_addNamespace (EcUdc, const EcString, const EcString definition);

__CPP_EXTERN______________________________________________________________________________END

#endif
