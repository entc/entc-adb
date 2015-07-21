/*
 * Copyright (c) 2010-2013 "Alexander Kalkhof" [email:adbo@kalkhof.org]
 *
 * This file is part of adbo framework (Advanced Database Objects)
 *
 * adbo is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * adbo is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with adbo. If not, see <http://www.gnu.org/licenses/>.
 */

#include "adbo_context.h"
#include "adbo_context_intern.h"

//----------------------------------------------------------------------------------------

AdboContext adbo_context_create (EcEventFiles files, const EcString configpath, const EcString execpath)
{
  AdboContext self = ENTC_NEW (struct AdboContext_s);
  
  self->adblm = adbl_new ();
  adbl_scan (self->adblm, files, configpath, execpath);  

 // self->substitutes = adbo_subsmgr_new (path, self);  
  
  return self;
}

//----------------------------------------------------------------------------------------

void adbo_context_destroy (AdboContext* pself)
{
  AdboContext self = *pself;
  
 // adbo_subsmgr_del (&(self->substitutes));
  adbl_delete (&(self->adblm));
  
  ENTC_DEL (pself, struct AdboContext_s);
}

//----------------------------------------------------------------------------------------
