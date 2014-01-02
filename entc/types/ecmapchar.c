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

#include "ecmapchar.h"

/*------------------------------------------------------------------------*/

struct EcMapChar_s
{
  EcList list;
};

struct EcMapCharDataNode
{
  EcString key;
  
  EcString value;
};

/*------------------------------------------------------------------------*/

EcMapChar ecmapchar_new()
{
  EcMapChar self = ENTC_NEW(struct EcMapChar_s);
  
  self->list = eclist_new();
  
  return self;
}

/*------------------------------------------------------------------------*/

void ecmapchar_delete(EcMapChar* pself)
{
  EcMapChar self = *pself;
  
  ecmapchar_clear(self);
  
  eclist_delete( &(self->list) );
  self->list = 0;
  
  ENTC_DEL(pself, struct EcMapChar_s);
}

/*------------------------------------------------------------------------*/

void ecmapchar_clear(EcMapChar self)
{
  /* iterate through all list members */
  EcListNode node;
  for(node = eclist_first(self->list); node != eclist_end(self->list); node = eclist_next(node))
  {
    struct EcMapCharDataNode* mapnode = eclist_data(node);
    
    ecstr_delete(&(mapnode->key));
    ecstr_delete(&(mapnode->value));
    
    ENTC_DEL(&mapnode, struct EcMapCharDataNode);
  }
  eclist_clear(self->list);  
}

/*------------------------------------------------------------------------*/

void ecmapchar_append(EcMapChar self, const EcString key, const EcString value)
{
  struct EcMapCharDataNode* mapnode = ENTC_NEW(struct EcMapCharDataNode);
  mapnode->key = ecstr_copy(key);
  mapnode->value = ecstr_copy(value);
  
  eclist_append(self->list, mapnode);
}

/*------------------------------------------------------------------------*/

void ecmapchar_appendTO(EcMapChar self, const EcString key, EcString value)
{
  struct EcMapCharDataNode* mapnode = ENTC_NEW(struct EcMapCharDataNode);
  
  mapnode->key = ecstr_copy(key);
  mapnode->value = value;
  
  eclist_append(self->list, mapnode); 
}

/*------------------------------------------------------------------------*/

void ecmapchar_set(EcMapChar self, const EcString key, const EcString value)
{
  EcListNode node = ecmapchar_find(self, key);
  
  if( node == eclist_end(self->list) )
  {
    ecmapchar_append(self, key, value);
  }
  else
  {
    struct EcMapCharDataNode* mapnode = eclist_data(node);
    
    mapnode->value = ecstr_replace( &(mapnode->value), value);
  }
}

/*------------------------------------------------------------------------*/

EcMapCharNode ecmapchar_find(EcMapChar self, const EcString key)
{
  /* iterate through all list members */
  EcListNode node;
  for(node = eclist_first(self->list); node != eclist_end(self->list); node = eclist_next(node))
  {
    struct EcMapCharDataNode* mapnode = (struct EcMapCharDataNode*)eclist_data(node);
    /* compare the stored name with the name we got */
    if(ecstr_equal(mapnode->key, key))
    {
      return node;
    }
  }
  return eclist_end(self->list);
}

/*------------------------------------------------------------------------*/

const EcString ecmapchar_finddata(EcMapChar self, const EcString key)
{
  /* iterate through all list members */
  EcListNode node;
  for(node = eclist_first(self->list); node != eclist_end(self->list); node = eclist_next(node))
  {
    struct EcMapCharDataNode* mapnode = (struct EcMapCharDataNode*)eclist_data(node);
    /* compare the stored name with the name we got */
    if (ecstr_equal(mapnode->key, key))
	{
      return mapnode->value;
	}
  }
  return 0;  
}

/*------------------------------------------------------------------------*/

EcMapCharNode ecmapchar_first(const EcMapChar self)
{
  return eclist_first(self->list);
}

/*------------------------------------------------------------------------*/

EcMapCharNode
ecmapchar_next(const EcMapCharNode node)
{
  return eclist_next((EcListNode)node);
}

/*------------------------------------------------------------------------*/

EcMapCharNode ecmapchar_end(const EcMapChar self)
{
  return eclist_end(self->list);
}

/*------------------------------------------------------------------------*/

const EcString ecmapchar_key(const EcMapCharNode node)
{
  struct EcMapCharDataNode* mapnode = eclist_data((EcListNode)node);
  return mapnode->key;  
}

/*------------------------------------------------------------------------*/

const EcString ecmapchar_data(const EcMapCharNode node)
{
  struct EcMapCharDataNode* mapnode = eclist_data((EcListNode)node);
  return mapnode->value;
}

/*------------------------------------------------------------------------*/

void ecmapchar_copy(EcMapChar dest, const EcMapChar source)
{
  /* create the iterator node */
  EcMapCharNode node;
  /* copy all elements into the new map */
  for(node = ecmapchar_first( source ); node != ecmapchar_end( source ); node = ecmapchar_next( node ))
    ecmapchar_append(dest, ecmapchar_key(node), ecmapchar_data(node));
}

/*------------------------------------------------------------------------*/

uint_t ecmapchar_count(const EcMapChar self)
{
  /* variables */
  uint_t counter = 0;
  /* count all entries */
  EcListNode node;
  for(node = eclist_first(self->list); node != eclist_end(self->list); node = eclist_next(node))
  {
    counter++;
  }
  
  return counter;
}

/*------------------------------------------------------------------------*/
