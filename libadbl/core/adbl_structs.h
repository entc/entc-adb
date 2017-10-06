/*
 * Copyright (c) 2010-2013 "Alexander Kalkhof" [email:adbl@kalkhof.org]
 *
 * This file is part of adbl framework (Advanced Database Layer)
 *
 * adbl is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * adbl is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with adbl. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef ADBL_STRUCTS_H
#define ADBL_STRUCTS_H

#include "types/ecmapchar.h"
#include "types/ecudc.h"

struct AdblSession_s;

typedef struct AdblSession_s* AdblSession;

//------------------------------------------

typedef struct 
{
  
  ubyte_t inicident;
  
} AdblSecurity;

//------------------------------------------

typedef struct 
{
  
  EcMapChar columns;
  
} AdblAttributes;

//------------------------------------------

typedef struct 
{
  
  ubyte_t type;
  
  EcList list;
  
} AdblConstraint;

//------------------------------------------

typedef struct 
{
  
  ubyte_t type;
  
  EcUdc data;
  
  AdblConstraint* constraint;    
  
} AdblConstraintElement;  

//------------------------------------------

// if table has a value then we have a subquery
typedef struct 
{
  
  EcString column;
  
  EcString table;
  
  EcString ref;
  
  EcString value;
  
  int orderno;
  
} AdblQueryColumn;

//------------------------------------------

typedef struct 
{
  
  EcString table;
  
  EcList columns;
  
  uint_t limit;
  
  uint_t offset;
  
  AdblConstraint* constraint;
  
} AdblQuery;

//------------------------------------------

typedef struct
{
  
  EcString procedure;
  
  EcUdc values;
  
} AdblProcedure;

//------------------------------------------

typedef struct 
{
  
  EcString table;
  
  AdblConstraint* constraint;
  
  AdblAttributes* attrs;
  
} AdblUpdate;

//------------------------------------------

typedef struct 
{
  
  EcString table;
  
  AdblAttributes* attrs;
  
} AdblInsert;

//------------------------------------------

typedef struct
{
  
  EcString table;
  
  AdblConstraint* constraint;
  
  int force_all;
  
} AdblDelete;

//------------------------------------------

struct AdblCursor_p;

typedef struct AdblCursor_p AdblCursor;

struct AdblSequence_p;

typedef struct AdblSequence_p AdblSequence;

typedef struct
{
  
  // refrence to AdblTable.name
  EcString name;

  EcString column_name;
  
  EcString table;
  
  EcString reference;
  
} AdblForeignKeyConstraint;

typedef struct
{
  
  EcString name;
  
  EcList columns;
  
  EcList primary_keys;
  
  EcList foreign_keys;
  
} AdblTable;

//------------------------------------------

#endif

