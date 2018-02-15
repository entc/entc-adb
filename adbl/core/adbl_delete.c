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

#include "adbl_delete.h"
#include "types/eclist.h"

#include "adbl_constraint.h"

//----------------------------------------------------------------------------------------

AdblDelete* adbl_delete_new (void)
{
  AdblDelete* self = ENTC_NEW(AdblDelete);
  
  self->table = 0;
  self->constraint = 0;
  self->force_all = FALSE;
  
  return self;
}

//----------------------------------------------------------------------------------------

void adbl_delete_delete (AdblDelete** ptr)
{
  AdblDelete* self = *ptr;
  
  adbl_delete_clear( self );
  
  ENTC_DEL( ptr, AdblDelete );
}

//----------------------------------------------------------------------------------------

void adbl_delete_clear (AdblDelete* self)
{
  ecstr_delete( &(self->table) );
}

//----------------------------------------------------------------------------------------

void adbl_delete_setTable (AdblDelete* self, const char* table)
{
  ecstr_replace(&(self->table), table);
}

//----------------------------------------------------------------------------------------

void adbl_delete_setConstraint (AdblDelete* self, AdblConstraint* constraint)
{
  self->constraint = constraint;
}

//----------------------------------------------------------------------------------------

void adbl_delete_sec (AdblDelete* self, AdblSecurity* security)
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
}

//----------------------------------------------------------------------------------------

void adbl_delete_setForce (AdblDelete* self, int force)
{
  self->force_all = force;
}

//----------------------------------------------------------------------------------------
