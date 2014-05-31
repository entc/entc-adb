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

#include "adbo.h"

struct AdboItem_s
{
  
  int dummy;
  
}; typedef struct AdboItem_s* AdboItem;

//----------------------------------------------------------------------------------------

EcUdc adbo_value_fromXml (AdboContext context, EcXMLStream xmlstream);

void adbo_item_fromXml (EcUdc item, AdboContext context, EcXMLStream xmlstream, const EcString tag)
{
  if (isAssigned (xmlstream))
  {
    ENTC_XMLSTREAM_BEGIN
    
    if (ecxmlstream_isBegin (xmlstream, "value"))
    {
      EcUdc object = adbo_value_fromXml (context, xmlstream);
      if (isAssigned (object))
      {
        ecudc_add (item, &object);      
      }
    }
    else if (ecxmlstream_isBegin (xmlstream, "data"))
    {
      
    }
    
    ENTC_XMLSTREAM_END (tag)
  }  
}

//----------------------------------------------------------------------------------------
