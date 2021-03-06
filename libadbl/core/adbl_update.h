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

#ifndef ADBL_UPDATE_H
#define ADBL_UPDATE_H 1

#include <system/macros.h>
#include <utils/eclogger.h>

#include "adbl_structs.h"

__CPP_EXTERN______________________________________________________________________________START
  
__LIB_EXPORT AdblUpdate* adbl_update_new (void);
  
__LIB_EXPORT void adbl_update_delete (AdblUpdate**);
  
__LIB_EXPORT void adbl_update_clear (AdblUpdate*);
  
__LIB_EXPORT void adbl_update_setTable (AdblUpdate*, const EcString table);
  
__LIB_EXPORT void adbl_update_setAttributes (AdblUpdate*, AdblAttributes*);
  
__LIB_EXPORT void adbl_update_setConstraint (AdblUpdate*, AdblConstraint*);
  
  // security methods 
__LIB_EXPORT void adbl_update_sec (AdblUpdate*, AdblSecurity*);
  
__CPP_EXTERN______________________________________________________________________________END

#endif

