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

#ifndef ADBL_MANAGER_H
#define ADBL_MANAGER_H 1

#include "adbl.h"


#include <types/ecstring.h>

struct AdblManager_s; typedef struct AdblManager_s* AdblManager;

//-----------------------------------------------------------------------------  

__ENTC_LIBEX AdblManager adbl_new ();
  
__ENTC_LIBEX void adbl_scanJson (AdblManager, const EcString configpath, const EcString execpath);

__ENTC_LIBEX void adbl_delete (AdblManager*);
  
__ENTC_LIBEX void adbl_setCredentialsFile (AdblManager, const EcString name, const EcString module, const EcString file);
  
__ENTC_LIBEX AdblSession adbl_openSession (AdblManager, const EcString dbsource);

__ENTC_LIBEX void adbl_closeSession (AdblSession*);
  
//-----------------------------------------------------------------------------

#endif

