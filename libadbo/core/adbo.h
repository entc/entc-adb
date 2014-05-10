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

#ifndef ADBO__H
#define ADBO__H 1

#include <system/macros.h>
#include <types/ecudc.h>

#include "adbo_types.h"
#include "adbo_value.h"

__CPP_EXTERN______________________________________________________________________________START

// creates the structure from xml
__LIB_EXPORT EcUdc adbo_structure_fromXml (AdboContext, EcXMLStream, const EcString name, const EcString tag);

// creates the structure from the current database schema
__LIB_EXPORT EcUdc adbo_structure_fromDatabase (AdboContext);

// get a table node
__LIB_EXPORT EcUdc adbo_get_table (EcUdc, const EcString tablename);

// fills the structure with the current content from the database
__LIB_EXPORT int adbo_fetch (EcUdc, EcUdc filter, AdboContext);

// writes back the content in the structure to the database
__LIB_EXPORT int adbo_update (EcUdc, EcUdc filter, AdboContext, EcUdc data);

// delete the content in the structure from database
__LIB_EXPORT int adbo_delete (EcUdc, EcUdc filter, AdboContext);

__CPP_EXTERN______________________________________________________________________________END

#endif
