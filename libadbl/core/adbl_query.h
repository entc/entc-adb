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

#ifndef ADBL_QUERY_H
#define ADBL_QUERY_H 1

#include <system/macros.h>
#include <utils/eclogger.h>

#include "adbl_structs.h"

__CPP_EXTERN______________________________________________________________________________START  
   
__LIB_EXPORT AdblQuery* adbl_query_new (void);
  
__LIB_EXPORT void adbl_query_delete (AdblQuery**);
  
__LIB_EXPORT void adbl_query_clear (AdblQuery*);
  
__LIB_EXPORT void adbl_query_setTable (AdblQuery*, const EcString table);
  
__LIB_EXPORT void adbl_query_setConstraint (AdblQuery*, AdblConstraint*);
  
__LIB_EXPORT void adbl_query_addColumn (AdblQuery*, const EcString column, int order_pos);
  
__LIB_EXPORT void adbl_query_addColumnAsSubquery (AdblQuery*, const EcString column, const EcString table, const EcString ref, const EcString value, int order_pos);
  
__LIB_EXPORT void adbl_query_setLimit (AdblQuery*, uint_t);

__LIB_EXPORT void adbl_query_setOffset (AdblQuery*, uint_t);
  
  // security methods
__LIB_EXPORT void adbl_query_sec (AdblQuery*, AdblSecurity*, EcLogger);
    
__CPP_EXTERN______________________________________________________________________________END

#endif

