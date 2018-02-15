/*
 * Copyright (c) 2010-2016 "Alexander Kalkhof" [email:adbl@kalkhof.org]
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

#include "adbl_attributes.h"
#include "adbl_structs.h"
#include "adbl_security.h"

#include <types/ecstring.h>
#include <types/ecmap.h>
#include <types/ecalloc.h>

#include <stdio.h>

//------------------------------------------------------------------------

static void __STDCALL adbl_attrs_onDestroy (void* key, void* val)
{
  {
    EcString h = key; ecstr_delete (&h);
  }
  {
    EcString h = val; ecstr_delete (&h);
  }
}

//------------------------------------------------------------------------

AdblAttributes* adbl_attrs_new (void)
{
  AdblAttributes* self = ENTC_NEW (AdblAttributes);
  
  self->columns = ecmap_create (NULL, adbl_attrs_onDestroy);
  
  return self;
}

//------------------------------------------------------------------------

void adbl_attrs_delete (AdblAttributes** ptr)
{
  AdblAttributes* self = *ptr;
  
  ecmap_destroy (&(self->columns));
  
  ENTC_DEL(ptr, AdblAttributes);
}

//------------------------------------------------------------------------

void adbl_attrs_clear (AdblAttributes* self)
{
  ecmap_clear (self->columns);
}

//------------------------------------------------------------------------

void adbl_attrs_addChar (AdblAttributes* self, const EcString column, const EcString value)
{
  ecmap_insert (self->columns, ecstr_copy (column), ecstr_copy(value));
}

//------------------------------------------------------------------------

void adbl_attrs_addLong (AdblAttributes* self, const EcString column, uint_t value)
{
  ecmap_insert (self->columns, ecstr_copy (column), ecstr_long(value));
}

//------------------------------------------------------------------------

const EcString adbl_attrs_get (AdblAttributes* self, const EcString column)
{
  EcMapNode node = ecmap_find (self->columns, (void*)column);
  
  return ecmap_node_value (node);
}

//------------------------------------------------------------------------

int adbl_attrs_empty (AdblAttributes* self)
{
  return ecmap_size (self->columns) == 0;
}

//------------------------------------------------------------------------

uint32_t adbl_attrs_size (AdblAttributes* self)
{
  return ecmap_size (self->columns);
}

//------------------------------------------------------------------------

void adbl_attrs_sec (AdblAttributes* self, AdblSecurity* security)
{
  //EcMapCharNode node;
  /*
  for( node = ecmapchar_first( self->columns ); node != ecmapchar_end( self->columns ); node = ecmapchar_next(node) )
  {
    
    if( adbl_security_checkValue( ecmapchar_key(node) ) )
    {
      security->inicident = 1;
      
      //eclogger_sec (logger, SL_RED);
      
      eclogger_fmt (LL_DEBUG, "ADBL", "security", "suspious content in column string '%s'", ecmapchar_key(node) );
      
      return;
    }
    if( adbl_security_checkValue( ecmapchar_data(node) ) )
    {
      security->inicident = 1;
      
      // eclogger_sec (logger, SL_RED);

      eclogger_fmt (LL_DEBUG, "ADBL", "security", "suspious content in value string '%s'", ecmapchar_data(node) );
      
      return;
    }
     
  }
   */
}

//------------------------------------------------------------------------
