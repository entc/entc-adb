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

#ifndef ADBL_DELETE_H
#define ADBL_DELETE_H 1

#include <system/macros.h>

#include "adbl_security.h"
#include "adbl_structs.h"

__CPP_EXTERN______________________________________________________________________________START

__LIB_EXPORT AdblDelete* adbl_delete_new();
  
__LIB_EXPORT void adbl_delete_delete (AdblDelete**);
  
__LIB_EXPORT void adbl_delete_clear (AdblDelete*);
  
__LIB_EXPORT void adbl_delete_setTable (AdblDelete*, const EcString table);
  
__LIB_EXPORT void adbl_delete_setConstraint (AdblDelete*, AdblConstraint*);
  
__LIB_EXPORT void adbl_delete_setForce (AdblDelete*, int);
  
  /* security methods */
__LIB_EXPORT void adbl_delete_sec (AdblDelete*, AdblSecurity*);

__CPP_EXTERN______________________________________________________________________________END

#endif

