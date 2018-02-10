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

AdblProcedure* adbl_procedure_create (const EcString procedureName)
{
  AdblProcedure* self = ENTC_NEW (AdblProcedure);
  
  self->procedure = ecstr_copy(procedureName);
  self->values = ecudc_create (EC_ALLOC, ENTC_UDC_LIST, NULL);
  
  return self;
}

//----------------------------------------------------------------------------------------

void adbl_procedure_destroy (AdblProcedure** pself)
{
  AdblProcedure* self = *pself;
  
  ecstr_delete(&(self->procedure));
  ecudc_destroy(EC_ALLOC, &(self->values));
  
  ENTC_DEL(pself, AdblProcedure);
}

//----------------------------------------------------------------------------------------

void adbl_procedure_addValue (AdblProcedure* self, EcUdc* val)
{
  ecudc_add (self->values, val);
}

//----------------------------------------------------------------------------------------
