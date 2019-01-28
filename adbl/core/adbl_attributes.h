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

#ifndef ADBL_ATTRIBUTES_H
#define ADBL_ATTRIBUTES_H 1

#include <sys/entc_export.h>
#include "adbl_structs.h"

__ENTC_LIBEX AdblAttributes* adbl_attrs_new (void);
  
__ENTC_LIBEX void adbl_attrs_delete (AdblAttributes**);
  
__ENTC_LIBEX void adbl_attrs_clear (AdblAttributes*);
  
__ENTC_LIBEX void adbl_attrs_addChar (AdblAttributes*, const EcString column, const EcString value);
  
__ENTC_LIBEX void adbl_attrs_addLong (AdblAttributes*, const EcString column, uint_t value);
  
__ENTC_LIBEX const EcString adbl_attrs_get (AdblAttributes*, const EcString column);
  
__ENTC_LIBEX int adbl_attrs_empty (AdblAttributes*);

__ENTC_LIBEX uint32_t adbl_attrs_size (AdblAttributes*);

  // security methods
__ENTC_LIBEX void adbl_attrs_sec (AdblAttributes*, AdblSecurity*);
  
#endif

