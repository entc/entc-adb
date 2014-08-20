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

#ifndef ENTC_TOOLS_DATA_H
#define ENTC_TOOLS_DATA_H 1

#include "../system/macros.h"
#include "../system/types.h"

#include "../types/ecudc.h"
#include "../types/ecstring.h"

#define ECDATA_ROOT   ".root"
#define ECDATA_NODES  ".nodes"
#define ECDATA_ITEMS  ".items"
#define ECDATA_SIZE   ".size"
#define ECDATA_CDATE  ".cdate"
#define ECDATA_MDATE  ".mdate"
#define ECDATA_NAME   ".name"
#define ECDATA_IDS    ".ids"
#define ECDATA_REFS   ".refs"
#define ECDATA_COLS   ".cols"
#define ECDATA_ROWS   ".rows"

__CPP_EXTERN______________________________________________________________________________START

// node methods

__LIB_EXPORT EcUdc ecnode_create (const EcString name);

__LIB_EXPORT void ecnode_set_attributes (EcUdc node, ulong_t size, ulong_t cdate, ulong_t mdate);

__LIB_EXPORT EcUdc ecnode_create_item (EcUdc node, const EcString name);

__LIB_EXPORT EcUdc ecnode_create_node (EcUdc node, const EcString name);

__CPP_EXTERN______________________________________________________________________________END

#endif
