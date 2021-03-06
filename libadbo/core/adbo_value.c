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

#include "adbo_value.h"

#include "adbo_object.h"
#include "adbo_container.h"

//----------------------------------------------------------------------------------------

EcUdc adbo_value_fromXml (AdboContext context, EcXMLStream xmlstream)
{
  EcUdc value = NULL;
  
  const EcString dbcolumn = ecxmlstream_nodeAttribute (xmlstream, "dbcolumn");
  if (dbcolumn)
  {
    const EcString size;

    value = ecudc_create (EC_ALLOC, ENTC_UDC_NODE, dbcolumn);
  
    // check for more options
    size = ecxmlstream_nodeAttribute (xmlstream, "size");
    if (size)
    {
      ecudc_add_asUInt32(EC_ALLOC, value, "size", atoi(size));
    }
  }
  
  return value;
}

//----------------------------------------------------------------------------------------

/*
struct AdboValue_s
{
  
  EcString column;
  
  EcString link;

  // under transaction control
    
  int state;
  
  int state_orig;

  EcString data;

  EcString data_orig;
  
};

//----------------------------------------------------------------------------------------

void adbo_value_add (EcUdc value, const EcString key, const EcString content)
{
  if (ecstr_empty(content))
  {
    return;
  }
  
  ecudc_add_asString(value, key, content);
}


//----------------------------------------------------------------------------------------

AdboValue adbo_value_new (const EcString dbcolumn, const EcString data, const EcString link)
{
  AdboValue self = ENTC_NEW (struct AdboValue_s);

  self->column = ecstr_copy (dbcolumn);
  self->link = ecstr_copy (link);

  if ( ecstr_valid (data))
  {
    self->data = ecstr_copy (data);
    self->state = ADBO_STATE_FIXED;
    
    self->data_orig = ecstr_copy (data);
    self->state_orig = ADBO_STATE_FIXED;
  }
  else
  {
    self->data = ecstr_init ();
    self->state = ADBO_STATE_NONE;
    
    self->data_orig = ecstr_init();
    self->state_orig = ADBO_STATE_NONE;
  }
  
  return self;
}

//----------------------------------------------------------------------------------------

AdboValue adbo_value_newFromXml (EcXMLStream xmlstream)
{
  return adbo_value_new (
    ecxmlstream_nodeAttribute (xmlstream, "dbcolumn"),
    ecxmlstream_nodeAttribute (xmlstream, "data"),
    ecxmlstream_nodeAttribute (xmlstream, "link")
                         );
}

//----------------------------------------------------------------------------------------

void adbo_value_del (AdboValue* pself)
{
  AdboValue self = *pself;
  
  ecstr_delete (&(self->column));
  ecstr_delete (&(self->link));
  
  ecstr_delete (&(self->data));
  ecstr_delete (&(self->data_orig));

  ENTC_DEL (pself, struct AdboValue_s);
}

//----------------------------------------------------------------------------------------
*/
