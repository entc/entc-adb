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

#include "adbl_security.h"

#include <string.h>

/*------------------------------------------------------------------------*/

int adbl_security_checkValue (const EcString value)
{
  if( !value )
    return FALSE;
  
  /* FIXME, make it more efficient */
  
  if( strchr ( value, ';' ) )
    return TRUE;
  
  if( strchr ( value, '"' ) )
    return TRUE;
  
  if( strchr ( value, '\'' ) )
    return TRUE;
  
  if( strchr ( value, '(' ) )
    return TRUE;
  
  if( strchr ( value, ')' ) )
    return TRUE;
  
  return FALSE;  
}

/*------------------------------------------------------------------------*/
