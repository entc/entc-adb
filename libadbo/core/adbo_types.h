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

#include <tools/ecxmlstream.h>
#include <utils/eclogger.h>
#include <types/ecstream.h>

//##########################################

#define ADBO_OBJECT_NONE        0
#define ADBO_OBJECT_NODE        1
#define ADBO_OBJECT_SUBSTITUTE  2
#define ADBO_OBJECT_ITEM        3

//##########################################

// the value is fixed defined
#define ADBO_STATE_FIXED       10

// the value is synced with the database
#define ADBO_STATE_ORIGINAL     1
// the value must be updated
#define ADBO_STATE_CHANGED      2
// the value must be delete from database
#define ADBO_STATE_DELETED     -1

// the value was not defined
#define ADBO_STATE_NONE         0
// the value is new and not in the database
#define ADBO_STATE_INSERTED     3

//##########################################

struct AdboValue_s; typedef struct AdboValue_s* AdboValue;

struct AdboContainer_s; typedef struct AdboContainer_s* AdboContainer;

// type definition
struct AdboObject_s; typedef struct AdboObject_s* AdboObject;

struct AdboSubManager_s; typedef struct AdboSubManager_s* AdboSubManager;

struct AdboSchema_s; typedef struct AdboSchema_s* AdboSchema;

typedef struct AdboContext_s* AdboContext;

#endif
