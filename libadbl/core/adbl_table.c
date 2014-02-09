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

//----------------------------------------------------------------------------------------

AdblTable* adbl_table_new (const EcString tablename)
{
  AdblTable* self = ENTC_NEW (AdblTable);
  
  self->name = ecstr_copy(tablename);
  self->columns = eclist_new ();
  self->primary_keys = eclist_new ();
  self->foreign_keys = eclist_new ();
  
  return self;
}

//----------------------------------------------------------------------------------------

void adbl_table_del (AdblTable** pself)
{
  AdblTable* self = *pself;
  // variables
  EcListNode node;
  
  ecstr_delete(&(self->name));
  
  for (node = eclist_first(self->columns); node != eclist_end(self->columns); node = eclist_next(node))
  {
    EcString column = eclist_data (node);
    ecstr_delete(&column);
  }
  
  eclist_delete(&(self->columns));
  
  for (node = eclist_first(self->primary_keys); node != eclist_end(self->primary_keys); node = eclist_next(node))
  {
    EcString prikey = eclist_data (node);
    ecstr_delete(&prikey);
  }
  
  eclist_delete(&(self->primary_keys));

  for (node = eclist_first(self->foreign_keys); node != eclist_end(self->foreign_keys); node = eclist_next(node))
  {
    AdblForeignKeyConstraint* fkconstraint = eclist_data (node);

    ENTC_DEL (&fkconstraint, AdblForeignKeyConstraint);
  }
  
  eclist_delete(&(self->foreign_keys));

  ENTC_DEL (pself, AdblTable);
}

//----------------------------------------------------------------------------------------

void adbl_schema_del (EcList* pself)
{
  // variables
  EcListNode node;

  EcList list = *pself;
  for (node = eclist_first(list); node != eclist_end(list); node = eclist_next(node))
  {
    EcString value = eclist_data(node);
    ecstr_delete(&value);
  }
  
  eclist_delete(pself);
}

//----------------------------------------------------------------------------------------

