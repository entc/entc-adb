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

__LIB_EXPORT int adbo_node_cursor (AdboContext context, EcCursor cursor, EcUdc node, EcUdc data);

__CPP_EXTERN______________________________________________________________________________END

#endif
