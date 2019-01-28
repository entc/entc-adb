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

#ifndef ADBL_CONSTRAINT_H
#define ADBL_CONSTRAINT_H 1

#include <sys/entc_export.h>
#include "adbl_structs.h"

#define QUOMADBL_CONSTRAINT_AND 0
#define QUOMADBL_CONSTRAINT_OR 1
#define QUOMADBL_CONSTRAINT_IN 2

#define QUOMADBL_CONSTRAINT_EQUAL 0

__ENTC_LIBEX AdblConstraint* adbl_constraint_new (ubyte_t type);

__ENTC_LIBEX void adbl_constraint_delete (AdblConstraint**);
  
__ENTC_LIBEX void adbl_constraint_clear (AdblConstraint*);
  
__ENTC_LIBEX void adbl_constraint_addChar (AdblConstraint*, const EcString column, ubyte_t type, const EcString value);
  
__ENTC_LIBEX void adbl_constraint_addLong (AdblConstraint*, const EcString column, ubyte_t type, uint_t value);
  
__ENTC_LIBEX void adbl_constraint_addConstraint (AdblConstraint*, AdblConstraint*);
  
__ENTC_LIBEX void adbl_constraint_attrs (AdblConstraint*, AdblAttributes*);

__ENTC_LIBEX int adbl_constraint_empty (AdblConstraint*);
  
  // security methods 
__ENTC_LIBEX void adbl_constraint_sec (AdblConstraint*, AdblSecurity*);

#endif

