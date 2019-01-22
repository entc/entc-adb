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

#include "adbl_constraint.h"

#include "adbl_structs.h"
#include "adbl_security.h"
#include "adbl_attributes.h"

#include <stdio.h>

// entc includes
#include <stc/entc_list.h>

//-----------------------------------------------------------------------------

static void __STDCALL adbl_constraint_list_onDestroy (void* ptr)
{
  AdblConstraintElement* element = ptr;
  
  ecudc_destroy (EC_ALLOC, &(element->data));
  
  ENTC_DEL(&element, AdblConstraintElement);
}

/*------------------------------------------------------------------------*/

AdblConstraint* adbl_constraint_new (ubyte_t type)
{
  AdblConstraint* self = ENTC_NEW(AdblConstraint);
  
  self->type = type;
  self->list = entc_list_new (adbl_constraint_list_onDestroy);
  
  return self;
}

/*------------------------------------------------------------------------*/

void adbl_constraint_delete (AdblConstraint** ptr)
{
  AdblConstraint* self = *ptr;
  
  adbl_constraint_clear(self);
  
  entc_list_del (&(self->list));
  
  ENTC_DEL( ptr, AdblConstraint );
}

/*------------------------------------------------------------------------*/

int adbl_constraint_empty (AdblConstraint* self)
{
  return entc_list_empty (self->list);
}

/*------------------------------------------------------------------------*/

void adbl_constraint_clear (AdblConstraint* self)
{
  entc_list_clr (self->list);
}

/*------------------------------------------------------------------------*/

void adbl_constraint_addChar (AdblConstraint* self, const EcString column, ubyte_t type, const EcString value)
{
  AdblConstraintElement* element = ENTC_NEW(AdblConstraintElement);
  
  entc_list_push_back (self->list, element);
  
  element->type = type;
  
  element->data = ecudc_create (EC_ALLOC, ENTC_UDC_STRING, column);
  ecudc_setS (element->data, value);
  
  //no constraint
  element->constraint = 0;
}

/*------------------------------------------------------------------------*/

void adbl_constraint_addLong (AdblConstraint* self, const EcString column, ubyte_t type, uint_t value)
{
	AdblConstraintElement* element = ENTC_NEW(AdblConstraintElement);

	entc_list_push_back (self->list, element);

	element->type = type;
  
  element->data = ecudc_create (EC_ALLOC, ENTC_UDC_NUMBER, column);
  ecudc_setNumber (element->data, value);

	/* no constraint */
	element->constraint = 0;  
}

/*------------------------------------------------------------------------*/

void adbl_constraint_addConstraint (AdblConstraint* self, AdblConstraint* value)
{
  AdblConstraintElement* element = ENTC_NEW(AdblConstraintElement);
  
  entc_list_push_back (self->list, element);
  
  element->type = 0;
  
  element->data = NULL;

  //set constraint
  element->constraint = value;    
}

/*------------------------------------------------------------------------*/

void adbl_constraint_attrs (AdblConstraint* self, AdblAttributes* attrs)
{
  EntcListCursor cursor;
  
  entc_list_cursor_init (self->list, &cursor, ENTC_DIRECTION_FORW);
  
  while (entc_list_cursor_next (&cursor))
  {
    AdblConstraintElement* element = entc_list_node_data (cursor.node);
    
    EcString val = ecudc_getString (element->data);
    
    adbl_attrs_addChar( attrs, ecudc_name(element->data), val );
    
    ecstr_delete(&val);
  }
}

/*------------------------------------------------------------------------*/

void adbl_constraint_sec (AdblConstraint* self, AdblSecurity* security)
{
  /*
  EntcListNode node;
  for(node = eclist_first(self->list); node != eclist_end(self->list); node = eclist_next(node))
  {
    //AdblConstraintElement* element = entc_list_node_data(node);
    
   
    if( adbl_security_checkValue( ecudc_name(element->data) ) )
    {
      security->inicident = 1;
      
      //eclogger_sec (SL_RED);
      
      eclogger_fmt (LL_DEBUG, "ADBL", "security", "suspious content in column string '%s'", ecudc_name(element->data) );
      return;  
    }
     */

    /*
    if( adbl_security_checkValue( element->value ) )
    {
      security->inicident = 1;

      //eclogger_sec (logger, SL_RED);

      eclogger_fmt (LL_DEBUG, "ADBL", "security", "suspious content in value string '%s'", element->value );
      
      return;  
    }
     
    
  }
     */
}

/*------------------------------------------------------------------------*/
