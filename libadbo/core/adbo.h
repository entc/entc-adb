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

// get all tables
__LIB_EXPORT EcUdc adbo_tables (EcUdc);

// get size/count(*) for all tables
__LIB_EXPORT void adbo_updateSize (EcUdc, AdboContext);

// get a table node
__LIB_EXPORT EcUdc adbo_get_table (EcUdc, const EcString tablename);

// fills the structure with the current content from the database
__LIB_EXPORT int adbo_item_fetch (EcUdc item, EcUdc filter, AdboContext);

__LIB_EXPORT int adbo_item_cursor (AdboContext, EcCursor, EcUdc item, EcUdc filter);

// gets back the structure with its first items result array
__LIB_EXPORT EcUdc adbo_item_values (EcUdc item);

// clears the first result array
__LIB_EXPORT int adbo_clear (EcUdc);

// writes back the content in the structure to the database
__LIB_EXPORT int adbo_update (EcUdc, EcUdc filter, AdboContext, EcUdc data);

// delete the content in the structure from database
__LIB_EXPORT int adbo_delete (EcUdc, EcUdc filter, AdboContext);

__CPP_EXTERN______________________________________________________________________________END

struct Adbo_s; typedef struct Adbo_s* Adbo;

__CPP_EXTERN______________________________________________________________________________START

__LIB_EXPORT Adbo adbo_create (const EcString confPath, const EcString binPath, const EcString objFile);

__LIB_EXPORT void adbo_destroy (Adbo*);

__LIB_EXPORT int adbo_db_update (Adbo, const EcString table, EcUdc* data, EcUdc caseNode);

__LIB_EXPORT EcUdc adbo_db_cursor (Adbo, const EcString table, EcUdc params);

__LIB_EXPORT EcUdc adbo_db_fetch (Adbo, const EcString table, EcUdc params);

__LIB_EXPORT int adbo_db_delete (Adbo, const EcString table, EcUdc params);

__LIB_EXPORT struct AdblManager_s* adbo_db_adbl (Adbo);

__CPP_EXTERN______________________________________________________________________________END

#endif
