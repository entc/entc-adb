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

#ifndef ADBO_ITEM_H
#define ADBO_ITEM_H 1

#include <system/macros.h>

#include "adbo_types.h"

struct AdboItem_s; typedef struct AdboItem_s* AdboItem;

__CPP_EXTERN______________________________________________________________________________START

// constructor
__LIB_EXPORT AdboItem adbo_item_new (AdboObject, AdboContainer, EcXMLStream, const EcString, EcLogger);

// constructor
__LIB_EXPORT AdboItem adbo_item_clone (AdboItem);

__LIB_EXPORT void adbo_item_strToStream (AdboObject, AdboItem, EcStream);

__LIB_EXPORT EcString adbo_item_str (AdboObject);

__LIB_EXPORT int adbo_item_is (AdboObject, const EcString link);

__LIB_EXPORT void adbo_item_dump (AdboObject, int tab, int le, EcBuffer, EcLogger);

__CPP_EXTERN______________________________________________________________________________END

#endif
