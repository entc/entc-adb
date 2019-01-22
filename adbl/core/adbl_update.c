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

#include "adbl_update.h"
#include "adbl_attributes.h"
#include "adbl_constraint.h"
#include "adbl_security.h"

#include <stc/entc_list.h>

//------------------------------------------------------------------------

AdblUpdate* adbl_update_new (void)
{
  AdblUpdate* self = ENTC_NEW(AdblUpdate);
  
  self->table = 0;
  self->constraint = 0;
  self->attrs = 0;
  
  return self;
}

//------------------------------------------------------------------------

void adbl_update_delete (AdblUpdate** ptr)
{
  AdblUpdate* self = *ptr;
  
  adbl_update_clear( self );
  
  ENTC_DEL( ptr, AdblUpdate );
}

//------------------------------------------------------------------------

void adbl_update_clear (AdblUpdate* self)
{
  ecstr_delete( &(self->table) );
  
  self->constraint = 0;
  self->attrs = 0;
}

//------------------------------------------------------------------------

void adbl_update_setTable (AdblUpdate* self, const char* table)
{
  ecstr_replace(&(self->table), table);
}

//------------------------------------------------------------------------

void adbl_update_setAttributes (AdblUpdate* self, AdblAttributes* attrs)
{
  self->attrs = attrs;
}

//------------------------------------------------------------------------

void adbl_update_setConstraint (AdblUpdate* self, AdblConstraint* constraint)
{
  self->constraint = constraint;
}

/*------------------------------------------------------------------------*/

void adbl_update_sec (AdblUpdate* self, AdblSecurity* security)
{
  security->inicident = 0;
  
  if( self->constraint )
  {
    adbl_constraint_sec (self->constraint, security);
  }
  
  if( security->inicident )
  {
    /* some incident happen, just return */
    return;  
  }

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
