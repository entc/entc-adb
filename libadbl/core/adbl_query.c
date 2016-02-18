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

#include "adbl_query.h"

#include "types/eclist.h"

#include "adbl_structs.h"
#include "adbl_security.h"
#include "adbl_constraint.h"

//------------------------------------------------------------------------

AdblQuery* adbl_query_new (void)
{
  AdblQuery* self = ENTC_NEW(AdblQuery);
  
  self->columns = eclist_create_ex (EC_ALLOC);
  
  self->table = ecstr_init();
  
  self->limit = 0;
  self->offset = 0;
  
  self->constraint = 0;
  
  return self;
}

//------------------------------------------------------------------------

void adbl_query_delete (AdblQuery** ptr)
{
  AdblQuery* self = *ptr;
  
  adbl_query_clear( self );
  
  eclist_free_ex (EC_ALLOC, &(self->columns));
  
  ENTC_DEL( ptr, AdblQuery );
}

//------------------------------------------------------------------------

void adbl_query_clear (AdblQuery* self)
{
  /* variables */
  EcListNode node;

  ecstr_delete( &(self->table) );

  /* clear all entries */
  for( node = eclist_first(self->columns); node != eclist_end(self->columns); node = eclist_next(node) )
  {
    AdblQueryColumn* qc = eclist_data(node);
    /* clear column */
    ecstr_delete( &(qc->column) );
    ecstr_delete( &(qc->table) );
    ecstr_delete( &(qc->ref) );
    ecstr_delete( &(qc->value) );
    
    ENTC_DEL( &qc, AdblQueryColumn );
  }
  
  eclist_clear(self->columns);
}

//------------------------------------------------------------------------

void adbl_query_setTable (AdblQuery* self, const EcString table)
{
  ecstr_replace( &(self->table), table);
}

//------------------------------------------------------------------------

void adbl_query_setConstraint (AdblQuery* self, AdblConstraint* constraint)
{
  self->constraint = constraint;
}

//------------------------------------------------------------------------

void adbl_query_addColumn (AdblQuery* self, const EcString column, int order_pos)
{
  /* create a new query_column object */
  AdblQueryColumn* qc = ENTC_NEW( AdblQueryColumn );
  /* init */
  qc->column = ecstr_copy(column);
  qc->table = 0;
  qc->ref = 0;
  qc->value = 0;
  qc->orderno = order_pos;
  /* add to list */
  eclist_append (self->columns, qc);
}

//------------------------------------------------------------------------

void adbl_query_addColumnAsSubquery (AdblQuery* self, const EcString column, const EcString table, const EcString ref, const EcString value, int order_pos)
{
  /* create a new query_column object */
  AdblQueryColumn* qc = ENTC_NEW( AdblQueryColumn );
  /* init */
  qc->column = ecstr_copy(column);
  qc->table = ecstr_copy(table);
  qc->ref = ecstr_copy(ref);
  qc->value = ecstr_copy(value);
  qc->orderno = order_pos;
  /* add to list */
  eclist_append (self->columns, qc);  
}

//------------------------------------------------------------------------

void adbl_query_setLimit (AdblQuery* self, uint_t limit)
{
  self->limit = limit;
}

//------------------------------------------------------------------------

void adbl_query_setOffset (AdblQuery* self, uint_t offset)
{
  self->offset = offset;
}

/*------------------------------------------------------------------------*/

void adbl_query_sec (AdblQuery* self, AdblSecurity* security)
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
  
  /* query related checks */  
}

/*------------------------------------------------------------------------*/
