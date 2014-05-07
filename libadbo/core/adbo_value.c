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

EcUdc adbo_value_fromXml (AdboContext context, EcXMLStream xmlstream)
{
  EcUdc value = ecudc_create(ENTC_UDC_NODE, ".val");
  
  adbo_value_add (value, ".dbcolumn", ecxmlstream_nodeAttribute (xmlstream, "dbcolumn"));
  adbo_value_add (value, ".link", ecxmlstream_nodeAttribute (xmlstream, "link"));
  
  return value;
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

AdboValue adbo_value_clone (const AdboValue oself)
{
  AdboValue self;
  if (isNotAssigned (oself))
  {
    return NULL;
  }
  
  self = ENTC_NEW (struct AdboValue_s);

  self->column = ecstr_copy (oself->column);
  self->link = ecstr_copy (oself->link);

  if (oself->state == ADBO_STATE_FIXED)
  {
    self->data = ecstr_copy (oself->data);
    self->state = ADBO_STATE_FIXED;

    self->data_orig = ecstr_copy (oself->data_orig);
    self->state_orig = ADBO_STATE_FIXED;
  }
  else
  {
    self->data = ecstr_init ();
    self->state = ADBO_STATE_NONE;

    self->data_orig = ecstr_init ();
    self->state_orig = ADBO_STATE_NONE;
  }

  return self;
}

//----------------------------------------------------------------------------------------

void adbo_value_set (AdboValue self, const EcString data, int state)
{
  if (self->state == ADBO_STATE_FIXED)
  {
    return;
  }

  if (ecstr_equal (data, self->data))
  {
    return;
  }

  ecstr_replace (&(self->data), data);

  if (state == ADBO_STATE_CHANGED && self->state == ADBO_STATE_NONE)
  {
    self->state = ADBO_STATE_INSERTED;
  }
  else
  {
    self->state = state;
  }
}

//----------------------------------------------------------------------------------------

AdboObject adbo_value_cobject (AdboValue self, AdboContainer container)
{
  return adbo_container_at (container, self->link);
}

//----------------------------------------------------------------------------------------

AdboValue adbo_value_cseek (AdboValue self, AdboContainer container)
{
  if (ecstr_valid (self->link))
  {
    AdboValue value = adbo_getValue (adbo_container_at (container, self->link));
    if (isAssigned(value))
    {
      return value;
    }
  }
  
  return self;  
}

//----------------------------------------------------------------------------------------

const EcString adbo_value_cget (AdboValue self, AdboContainer container)
{
  AdboValue value = adbo_value_cseek (self, container);
  if (isAssigned (value))
  {
    return value->data;
  }
  return ecstr_init();  
}

//----------------------------------------------------------------------------------------

AdboValue adbo_value_seek (AdboValue self, AdboObject obj)
{
  if (ecstr_valid (self->link))
  {
    AdboValue value = adbo_getValue (adbo_at (obj, self->link));
    if (isAssigned(value))
    {
      return value;
    }
  }

  return self;
}

//----------------------------------------------------------------------------------------

const EcString adbo_value_get (AdboValue self, AdboObject obj)
{
  AdboValue value = adbo_value_seek (self, obj);
  if (isAssigned (value))
  {
    return value->data;
  }
  return ecstr_init();
}

//----------------------------------------------------------------------------------------

const EcString adbo_value_getDBColumn (AdboValue self)
{
  return self->column;
}

//----------------------------------------------------------------------------------------

const EcString adbo_value_getData (AdboValue self)
{
  return self->data;
}

//----------------------------------------------------------------------------------------

int adbo_value_getState (AdboValue self)
{
  return self->state;
}

//----------------------------------------------------------------------------------------

int adbo_value_hasLocalLink (AdboValue self)
{
  if (!ecstr_valid (self->link))
  {
    return FALSE;
  }
  
  if (ecstr_has (self->link, '/'))
  {
    return FALSE;
  }
  
  return TRUE;
}

//----------------------------------------------------------------------------------------

void adbo_value_commit (AdboValue self)
{
  self->state_orig = self->state;
  ecstr_replace(&(self->data_orig), self->data);
}

//----------------------------------------------------------------------------------------

void adbo_value_rollback (AdboValue self)
{
  self->state = self->state_orig;
  ecstr_replace(&(self->data), self->data_orig);  
}

//----------------------------------------------------------------------------------------
