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

#include "adbl_insert.h"
#include "types/eclist.h"

#include "adbl_attributes.h"

//------------------------------------------------------------------------

AdblInsert* adbl_insert_new (void)
{
  AdblInsert* self = ENTC_NEW(AdblInsert);
  
  self->table = 0;
  self->attrs = 0;
  
  return self;
}

//------------------------------------------------------------------------

void adbl_insert_delete (AdblInsert** ptr)
{
  AdblInsert* self = *ptr;
  
  adbl_insert_clear( self );
  
  ENTC_DEL( ptr, AdblInsert );
}

//------------------------------------------------------------------------

void adbl_insert_clear (AdblInsert* self)
{
  ecstr_delete( &(self->table) );
}

//------------------------------------------------------------------------

void adbl_insert_setTable (AdblInsert* self, const EcString table)
{
  ecstr_replace( &(self->table), table);
}

//------------------------------------------------------------------------

void adbl_insert_setAttributes (AdblInsert* self, AdblAttributes* attr)
{
  self->attrs = attr;
}

/*------------------------------------------------------------------------*/

void adbl_insert_sec (AdblInsert* self, AdblSecurity* security)
{
  if( !security )
  {
    return;  
  }
  
  security->inicident = 0;
  
  if( self->attrs )
  {
    adbl_attrs_sec (self->attrs, security);
  }
  
  if( security->inicident )
  {
    /* some incident happen, just return */
    return;  
  }
}

/*------------------------------------------------------------------------*/
