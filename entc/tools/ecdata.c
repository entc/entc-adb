/*
 * Copyright (c) 2010-2013 "Alexander Kalkhof" [email:entc@kalkhof.org]
 *
 * This file is part of the extension n' tools (entc-base) framework for C.
 *
 * entc-base is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * entc-base is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with entc-base.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "ecdata.h"

//-----------------------------------------------------------------------------------------------------------

void ecnode_add_default (EcUdc self)
{
  time_t t = 0;
  
  ecudc_add_asUInt64 (self, ECDATA_SIZE, 0);
  ecudc_add_asTime (self, ECDATA_CDATE, &t);
  ecudc_add_asTime (self, ECDATA_MDATE, &t);
}

//-----------------------------------------------------------------------------------------------------------

EcUdc ecnode_create (const EcString name)
{
  EcUdc self = ecudc_create (ENTC_UDC_NODE, name);
  
  EcUdc nodes = ecudc_create (ENTC_UDC_NODE, ECDATA_NODES);
  EcUdc items = ecudc_create (ENTC_UDC_NODE, ECDATA_ITEMS);
  
  ecudc_add(self, &nodes);
  ecudc_add(self, &items);
  
  ecnode_add_default (self);
  
  return self;
}

//-----------------------------------------------------------------------------------------------------------

void ecnode_set_attributes (EcUdc node, uint64_t size, const time_t* cdate, const time_t* mdate)
{
  {
    EcUdc h = ecudc_node (node, ECDATA_SIZE);
    if (isAssigned (h))
    {
      ecudc_setUInt64 (h, size);
    }
    else
    {
      ecudc_add_asUInt64 (node, ECDATA_SIZE, size);    
    }
  }
  {
    EcUdc h = ecudc_node (node, ECDATA_CDATE);
    if (isAssigned (h))
    {
      ecudc_setTime (h, cdate);
    }
    else
    {
      ecudc_add_asTime (node, ECDATA_CDATE, cdate);    
    }
  }
  {
    EcUdc h = ecudc_node (node, ECDATA_MDATE);
    if (isAssigned (h))
    {
      ecudc_setTime (h, mdate);
    }
    else
    {
      ecudc_add_asTime (node, ECDATA_MDATE, mdate);    
    }
  }
}

//-----------------------------------------------------------------------------------------------------------

EcUdc ecnode_create_item (EcUdc node, const EcString name)
{
  EcUdc self = ecudc_create (ENTC_UDC_NODE, name);
  
  ecnode_add_default (self);  

  {
    EcUdc h = self;    
    ecudc_add (ecudc_node (node, ECDATA_ITEMS), &h);
  }
  
  return self;  
}

//-----------------------------------------------------------------------------------------------------------

EcUdc ecnode_create_node (EcUdc node, const EcString name)
{
  EcUdc self = ecudc_create (ENTC_UDC_NODE, name);

  EcUdc nodes = ecudc_create (ENTC_UDC_NODE, ECDATA_NODES);
  EcUdc items = ecudc_create (ENTC_UDC_NODE, ECDATA_ITEMS);
  
  ecudc_add(self, &nodes);
  ecudc_add(self, &items);
  
  ecnode_add_default (self);  
  
  {
    EcUdc h = self;    
    ecudc_add (ecudc_node (node, ECDATA_NODES), &h);
  }
  
  return self;
}

//-----------------------------------------------------------------------------------------------------------

