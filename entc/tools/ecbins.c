/*
 * Copyright (c) 2010-2015 "Alexander Kalkhof" [email:entc@kalkhof.org]
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

#include "ecbins.h"
#include "utils/eclogger.h"

#define ENTC_BINSTYPE_STRING    0
#define ENTC_BINSTYPE_VECTOR    1
#define ENTC_BINSTYPE_CHAR      2
#define ENTC_BINSTYPE_UCHAR     3
#define ENTC_BINSTYPE_SHORT     4
#define ENTC_BINSTYPE_USHORT    5
#define ENTC_BINSTYPE_INT       6
#define ENTC_BINSTYPE_UINT      7
#define ENTC_BINSTYPE_DOUBLE    8
#define ENTC_BINSTYPE_FLOAT     9
#define ENTC_BINSTYPE_VEC2      10
#define ENTC_BINSTYPE_VEC3      11
#define ENTC_BINSTYPE_VEC4      12
#define ENTC_BINSTYPE_QUAT      13
#define ENTC_BINSTYPE_MAT2      14
#define ENTC_BINSTYPE_MAT3      15
#define ENTC_BINSTYPE_MAT4      16

//-----------------------------------------------------------------------------------------------------------

EcUdc ecbins_read (const EcBuffer buf, const EcString name)
{

}

//-----------------------------------------------------------------------------------------------------------

void ecbins_writeNode (EcStream stream, const EcUdc udc)
{
  
  
}

//-----------------------------------------------------------------------------------------------------------

void ecbins_writeList (EcStream stream, const EcUdc udc)
{
  
  
}

//-----------------------------------------------------------------------------------------------------------

void ecbins_writeString (EcStream stream, const EcUdc udc)
{
  const EcString s = ecudc_asString(udc);
  if (isNotAssigned (s))
  {
    return;
  }
  
  // write datatype identifier
  ecstream_appendc(stream, ENTC_BINSTYPE_STRING);

  // depending on the length of the string handle differently
  unsigned long len = ecstr_len(s);
  if (len < 256)
  {
    ecstream_appendc(stream, 1);
    
    ecstream_appendc(stream, len);
    
    ecstream_appendd(stream, s, len);
  }
  else if (len < 32000)  // can't fit into a single char
  {
    ecstream_appendc(stream, 2);
    
    uint16_t h = len;
    ecstream_appendd(stream, (const char*)&h, 2);

    ecstream_appendd(stream, s, len);
  }
  else
  {
    ecstream_appendc(stream, 3);
    
    uint32_t h = len;
    ecstream_appendd(stream, (const char*)&h, 4);
    
    ecstream_appendd(stream, s, len);
  }
}

//-----------------------------------------------------------------------------------------------------------

void ecbins_writeByte (EcStream stream, const EcUdc udc)
{
  char c = ecudc_asB(udc);
  
  // write datatype identifier
  ecstream_appendc(stream, 2);
  ecstream_appendc(stream, c);  
}

//-----------------------------------------------------------------------------------------------------------

void ecbins_writeUInt32 (EcStream stream, const EcUdc udc)
{
  uint32_t h = ecudc_asUInt32(udc);
  
  // write datatype identifier
  ecstream_appendc(stream, 3);
  ecstream_appendd(stream, (const char*)&h, 4);    
}

//-----------------------------------------------------------------------------------------------------------

void ecbins_writeUInt64 (EcStream stream, const EcUdc udc)
{
  uint64_t h = ecudc_asUInt64(udc);
  
  // write datatype identifier
  ecstream_appendc(stream, 4);
  ecstream_appendd(stream, (const char*)&h, 8);    
}  
  
//-----------------------------------------------------------------------------------------------------------

void ecbins_writeElement (EcStream stream, const EcUdc udc)
{
  switch (ecudc_type(udc))
  {
    case ENTC_UDC_NODE:
    {
      ecbins_writeNode (stream, udc);
    }
    break;
    case ENTC_UDC_LIST:
    {
      ecbins_writeList (stream, udc);
    }
    break;
    case ENTC_UDC_STRING:
    {
      ecbins_writeString (stream, udc);
    }
    break;
    case ENTC_UDC_BYTE:
    {
      ecbins_writeByte (stream, udc);
    }
    break;
    case ENTC_UDC_UINT32:
    {
      ecbins_writeUInt32 (stream, udc);
    }
    break;
    case ENTC_UDC_UINT64:
    {
      ecbins_writeUInt64 (stream, udc);
    }
    break;
    case ENTC_UDC_REF:
    {
      
    }
    break;
    case ENTC_UDC_TIME:
    {
      
    }
    break;
    case ENTC_UDC_CURSOR:
    {
      
    }
    break;
    case ENTC_UDC_FILEINFO:
    {
      
    }
    break;
    case ENTC_UDC_TABLEINFO:
    {
      
    }
    break;
    case ENTC_UDC_SET:
    {
      
    }
    break;
    case ENTC_UDC_USERINFO:
    {
      
    }
    break;
  }
}

//-----------------------------------------------------------------------------------------------------------

EcBuffer ecbins_write (const EcUdc udc)
{
  EcStream stream = ecstream_new();
  
  
  
  return ecstream_trans(&stream);
}

//-----------------------------------------------------------------------------------------------------------
