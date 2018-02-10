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

#include "adbl_table.h"

//-----------------------------------------------------------------------------

static int __STDCALL adbl_table_columns_onDestroy (void* ptr)
{
  EcString column = ptr;
  
  ecstr_delete (&column);

  return 0;
}

//-----------------------------------------------------------------------------

static int __STDCALL adbl_table_prikeys_onDestroy (void* ptr)
{
  EcString prikey = ptr;

  ecstr_delete (&prikey);

  return 0;
}

//-----------------------------------------------------------------------------

static int __STDCALL adbl_table_forkeys_onDestroy (void* ptr)
{
  AdblForeignKeyConstraint* fkconstraint = ptr;
  
  ENTC_DEL (&fkconstraint, AdblForeignKeyConstraint);

  return 0;
}

//----------------------------------------------------------------------------------------

AdblTable* adbl_table_new (const EcString tablename)
{
  AdblTable* self = ENTC_NEW (AdblTable);
  
  self->name = ecstr_copy (tablename);
  self->columns = eclist_create (adbl_table_columns_onDestroy);
  self->primary_keys = eclist_create (adbl_table_prikeys_onDestroy);
  self->foreign_keys = eclist_create (adbl_table_forkeys_onDestroy);
  
  return self;
}

//----------------------------------------------------------------------------------------

void adbl_table_del (AdblTable** pself)
{
  AdblTable* self = *pself;
  
  ecstr_delete (&(self->name));
  
  eclist_destroy (&(self->columns));
  
  eclist_destroy (&(self->primary_keys));
  
  eclist_destroy (&(self->foreign_keys));

  ENTC_DEL (pself, AdblTable);
}

//----------------------------------------------------------------------------------------

