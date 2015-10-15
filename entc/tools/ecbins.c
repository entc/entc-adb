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
#define ENTC_BINSTYPE_MAP       17
#define ENTC_BINSTYPE_CHAR      2
#define ENTC_BINSTYPE_UCHAR     3
#define ENTC_BINSTYPE_SHORT     4
#define ENTC_BINSTYPE_USHORT    5
#define ENTC_BINSTYPE_INT       6
#define ENTC_BINSTYPE_UINT      7
#define ENTC_BINSTYPE_LONG      18
#define ENTC_BINSTYPE_ULONG     19
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

EcString ecbins_readEcString (EcBuffer posbuf)
{
  EcString s;
  unsigned char stype;  
  unsigned long size;

  if (posbuf->size < 1)
  {
    return NULL;
  }
  // read size type
  stype = *(posbuf->buffer);
  
  size = 0;
  
  posbuf->buffer++;
  posbuf->size--;
  
  switch (stype)
  {
    case 1:
    {
      if (posbuf->size < 1)
      {
        return NULL;
      }
      
      size = (unsigned char)*(posbuf->buffer);
      
      posbuf->buffer++;
      posbuf->size--;
    }
    break;
    case 2:
    {
      if (posbuf->size < 2)
      {
        return NULL;
      }
      
      size = (uint16_t)*(posbuf->buffer);
      
      posbuf->buffer += 2;
      posbuf->size -= 2;
    }
    break;
    case 3:
    {
      if (posbuf->size < 4)
      {
        return NULL;
      }
      
      size = (uint32_t)*(posbuf->buffer);
      
      posbuf->buffer += 4;
      posbuf->size -= 4;
    }
    break;
  }
  
  if (posbuf->size < size)
  {
    return NULL;
  }
  
  s = ecstr_part((const char*)(posbuf->buffer), size);
  
  posbuf->buffer += size;
  posbuf->size -= size;

  return s;
}

//-----------------------------------------------------------------------------------------------------------

EcUdc ecbins_readString (EcBuffer posbuf, const EcString name)
{
  EcString s = ecbins_readEcString (posbuf);
  
  if (isAssigned (s))
  {
    EcUdc udc = ecudc_create(ENTC_UDC_STRING, name);

    ecudc_setS_o (udc, &s);

    return udc;
  }
  else
  {
    return NULL;
  }
}

//-----------------------------------------------------------------------------------------------------------

EcUdc ecbins_readList (EcBuffer posbuf, const EcString name)
{
  EcUdc udc;
  uint32_t i, size;
  
  if (posbuf->size < 4)
  {
    return NULL;
  }
  
  udc = ecudc_create(ENTC_UDC_LIST, name);
  
  size = (uint32_t)*(posbuf->buffer);
  
  posbuf->buffer += 4;
  posbuf->size -= 4;

  for (i = 0; i < size; i++)
  {
    EcUdc h = ecbins_readBuffer (posbuf, NULL);
    
    ecudc_add(udc, &h);
  }
  
  return udc;
}

//-----------------------------------------------------------------------------------------------------------

EcUdc ecbins_readNode (EcBuffer posbuf, const EcString name)
{
  EcUdc udc;
  uint32_t i, size;
  
  if (posbuf->size < 4)
  {
    return NULL;
  }
  
  udc = ecudc_create(ENTC_UDC_NODE, name);
  
  size = (uint32_t)*(posbuf->buffer);
  
  posbuf->buffer += 4;
  posbuf->size -= 4;

  for (i = 0; i < size; i++)
  {
    EcUdc h;
    EcString s = ecbins_readEcString (posbuf);
    
    h = ecbins_readBuffer (posbuf, s);
    
    ecudc_add(udc, &h);
    ecstr_delete(&s);
  }
  
  return udc;
}

//-----------------------------------------------------------------------------------------------------------

EcUdc ecbins_readByte (EcBuffer posbuf, const EcString name)
{
  if (posbuf->size < 1)
  {
    return NULL;    
  }
  else
  {
    EcUdc udc = ecudc_create(ENTC_UDC_BYTE, name);
    
    ecudc_setByte(udc, (byte_t)*(posbuf->buffer));
    
    posbuf->buffer += 1;
    posbuf->size -= 1;

    return udc;
  }
}

//-----------------------------------------------------------------------------------------------------------

EcUdc ecbins_readUByte (EcBuffer posbuf, const EcString name)
{
  if (posbuf->size < 1)
  {
    return NULL;    
  }
  else
  {
    EcUdc udc = ecudc_create(ENTC_UDC_UBYTE, name);
    
    ecudc_setUByte(udc, (ubyte_t)*(posbuf->buffer));
    
    posbuf->buffer += 1;
    posbuf->size -= 1;
    
    return udc;
  }
}

//-----------------------------------------------------------------------------------------------------------

EcUdc ecbins_readInt16 (EcBuffer posbuf, const EcString name)
{
  if (posbuf->size < 2)
  {
    return NULL;    
  }
  else
  {
    EcUdc udc = ecudc_create(ENTC_UDC_INT16, name);
    
    ecudc_setInt16(udc, (int16_t)*(posbuf->buffer));
    
    posbuf->buffer += 2;
    posbuf->size -= 2;
    
    return udc;
  }
}

//-----------------------------------------------------------------------------------------------------------

EcUdc ecbins_readUInt16 (EcBuffer posbuf, const EcString name)
{
  if (posbuf->size < 2)
  {
    return NULL;    
  }
  else
  {
    EcUdc udc = ecudc_create(ENTC_UDC_UINT16, name);
    
    ecudc_setUInt16(udc, (uint16_t)*(posbuf->buffer));
    
    posbuf->buffer += 2;
    posbuf->size -= 2;
    
    return udc;
  }
}

//-----------------------------------------------------------------------------------------------------------

EcUdc ecbins_readInt32 (EcBuffer posbuf, const EcString name)
{
  if (posbuf->size < 4)
  {
    return NULL;    
  }
  else
  {
    EcUdc udc = ecudc_create(ENTC_UDC_INT32, name);
    
    ecudc_setInt32(udc, (int32_t)*(posbuf->buffer));
    
    posbuf->buffer += 4;
    posbuf->size -= 4;
    
    return udc;
  }
}

//-----------------------------------------------------------------------------------------------------------

EcUdc ecbins_readUInt32 (EcBuffer posbuf, const EcString name)
{
  if (posbuf->size < 4)
  {
    return NULL;    
  }
  else
  {
    EcUdc udc = ecudc_create(ENTC_UDC_UINT32, name);
    
    ecudc_setUInt32(udc, (uint32_t)*(posbuf->buffer));
    
    posbuf->buffer += 4;
    posbuf->size -= 4;
    
    return udc;
  }
}

//-----------------------------------------------------------------------------------------------------------

EcUdc ecbins_readInt64 (EcBuffer posbuf, const EcString name)
{
  if (posbuf->size < 8)
  {
    return NULL;    
  }
  else
  {
    EcUdc udc = ecudc_create(ENTC_UDC_INT64, name);
    
    ecudc_setInt64(udc, (int64_t)*(posbuf->buffer));
    
    posbuf->buffer += 8;
    posbuf->size -= 8;
    
    return udc;
  }
}

//-----------------------------------------------------------------------------------------------------------

EcUdc ecbins_readUInt64 (EcBuffer posbuf, const EcString name)
{
  if (posbuf->size < 8)
  {
    return NULL;    
  }
  else
  {
    EcUdc udc = ecudc_create(ENTC_UDC_UINT64, name);
    
    ecudc_setUInt64(udc, (uint64_t)*(posbuf->buffer));
    
    posbuf->buffer += 8;
    posbuf->size -= 8;
    
    return udc;
  }
}

//-----------------------------------------------------------------------------------------------------------

EcUdc ecbins_readFloat (EcBuffer posbuf, const EcString name)
{
  if (posbuf->size < 4)
  {
    return NULL;    
  }
  else
  {
    EcUdc udc = ecudc_create(ENTC_UDC_FLOAT, name);
    
    ecudc_setFloat(udc, (float)*(posbuf->buffer));
    
    posbuf->buffer += 4;
    posbuf->size -= 4;
    
    return udc;
  }
}

//-----------------------------------------------------------------------------------------------------------

EcUdc ecbins_readDouble (EcBuffer posbuf, const EcString name)
{
  if (posbuf->size < 8)
  {
    return NULL;    
  }
  else
  {
    EcUdc udc = ecudc_create(ENTC_UDC_DOUBLE, name);
    
    ecudc_setDouble(udc, (double)*(posbuf->buffer));
    
    posbuf->buffer += 8;
    posbuf->size -= 8;
    
    return udc;
  }
}

//-----------------------------------------------------------------------------------------------------------

EcUdc ecbins_readBuffer (EcBuffer posbuf, const EcString name)
{
  unsigned char type;

  if (posbuf->size < 1)
  {
    return NULL;    
  }
  
  // read first char
  type = *(posbuf->buffer);
  
  posbuf->buffer++;
  posbuf->size--;

  switch (type)
  {
    case ENTC_BINSTYPE_STRING:
    {
      return ecbins_readString (posbuf, name);
    }
    case ENTC_BINSTYPE_VECTOR:
    {
      return ecbins_readList (posbuf, name);
    }
    case ENTC_BINSTYPE_MAP:
    {
      return ecbins_readNode (posbuf, name);
    }
    case ENTC_BINSTYPE_CHAR:
    {
      return ecbins_readByte (posbuf, name);
    }
    case ENTC_BINSTYPE_UCHAR:
    {
      return ecbins_readUByte (posbuf, name);
    }
    case ENTC_BINSTYPE_SHORT:
    {
      return ecbins_readInt16 (posbuf, name);
    }
    case ENTC_BINSTYPE_USHORT:
    {
      return ecbins_readUInt16 (posbuf, name);
    }
    case ENTC_BINSTYPE_INT:
    {
      return ecbins_readInt32 (posbuf, name);
    }
    case ENTC_BINSTYPE_UINT:
    {
      return ecbins_readUInt32 (posbuf, name);
    }
    case ENTC_BINSTYPE_LONG:
    {
      return ecbins_readInt64 (posbuf, name);
    }
    case ENTC_BINSTYPE_ULONG:
    {
      return ecbins_readUInt64 (posbuf, name);
    }
    case ENTC_BINSTYPE_FLOAT:
    {
      return ecbins_readFloat (posbuf, name);
    }
    case ENTC_BINSTYPE_DOUBLE:
    {
      return ecbins_readDouble (posbuf, name);
    }
  }
  
  return NULL;
}

//-----------------------------------------------------------------------------------------------------------

EcUdc ecbins_read (const EcBuffer buf, const EcString name)
{
  EcBuffer_s posbuf;
  
  posbuf.buffer = buf->buffer;
  posbuf.size = buf->size;
  
  return ecbins_readBuffer (&posbuf, name);
}

//-----------------------------------------------------------------------------------------------------------

void ecbins_writeEcString (EcStream stream, const EcString s)
{
  // depending on the length of the string handle differently
  unsigned long len = ecstr_len(s);
  if (len < 256)
  {
    ecstream_appendc(stream, 1);
    ecstream_appendc(stream, len);    
    ecstream_appendd(stream, s, len);
  }
  else if (len < 65536)  // can't fit into a single char
  {
    uint16_t h = len;

    ecstream_appendc(stream, 2);    
    ecstream_appendd(stream, (const char*)&h, 2);
    ecstream_appendd(stream, s, len);
  }
  else
  {
    uint32_t h = len;

    ecstream_appendc(stream, 3);
    ecstream_appendd(stream, (const char*)&h, 4);
    ecstream_appendd(stream, s, len);
  }  
}

//-----------------------------------------------------------------------------------------------------------

void ecbins_writeNode (EcStream stream, const EcUdc udc)
{
  void* cursor = NULL;
  EcUdc item;
  
  // write datatype identifier
  ecstream_appendc(stream, ENTC_BINSTYPE_MAP);
  
  {
    ulong_t offsetSize = ecstream_registerOffset (stream, 4);
    uint32_t counter = 0;
    
    for (item = ecudc_next(udc, &cursor); isAssigned (item); item = ecudc_next(udc, &cursor))
    {
      ecbins_writeEcString (stream, ecudc_name(item));
      
      ecbins_writeElement (stream, item);
      counter ++;
    }
    
    ecstream_fillOffset (stream, offsetSize, (const char*)&counter, 4);
  }
}

//-----------------------------------------------------------------------------------------------------------

void ecbins_writeList (EcStream stream, const EcUdc udc)
{
  void* cursor = NULL;
  EcUdc item;
  
  // write datatype identifier
  ecstream_appendc(stream, ENTC_BINSTYPE_VECTOR);

  {
    ulong_t offsetSize = ecstream_registerOffset (stream, 4);
    uint32_t counter = 0;
    
    for (item = ecudc_next(udc, &cursor); isAssigned (item); item = ecudc_next(udc, &cursor))
    {
      ecbins_writeElement (stream, item);
      counter ++;
    }
    
    ecstream_fillOffset (stream, offsetSize, (const char*)&counter, 4);
  }
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

  ecbins_writeEcString (stream, s);
}

//-----------------------------------------------------------------------------------------------------------

void ecbins_writeByte (EcStream stream, const EcUdc udc)
{
  byte_t c = ecudc_asByte(udc);
  
  // write datatype identifier
  ecstream_appendc(stream, ENTC_BINSTYPE_CHAR);
  ecstream_appendc(stream, c);  
}

//-----------------------------------------------------------------------------------------------------------

void ecbins_writeUByte (EcStream stream, const EcUdc udc)
{
  ubyte_t c = ecudc_asUByte(udc);
  
  // write datatype identifier
  ecstream_appendc(stream, ENTC_BINSTYPE_UCHAR);
  ecstream_appendc(stream, c);  
}

//-----------------------------------------------------------------------------------------------------------

void ecbins_writeInt16 (EcStream stream, const EcUdc udc)
{
  int16_t h = ecudc_asInt16(udc);
  
  // write datatype identifier
  ecstream_appendc(stream, ENTC_BINSTYPE_SHORT);
  ecstream_appendd(stream, (const char*)&h, 2);    
}

//-----------------------------------------------------------------------------------------------------------

void ecbins_writeUInt16 (EcStream stream, const EcUdc udc)
{
  uint16_t h = ecudc_asUInt16(udc);
  
  // write datatype identifier
  ecstream_appendc(stream, ENTC_BINSTYPE_USHORT);
  ecstream_appendd(stream, (const char*)&h, 2);    
}

//-----------------------------------------------------------------------------------------------------------

void ecbins_writeInt32 (EcStream stream, const EcUdc udc)
{
  int32_t h = ecudc_asInt32(udc);
  
  // write datatype identifier
  ecstream_appendc(stream, ENTC_BINSTYPE_INT);
  ecstream_appendd(stream, (const char*)&h, 4);    
}

//-----------------------------------------------------------------------------------------------------------

void ecbins_writeUInt32 (EcStream stream, const EcUdc udc)
{
  uint32_t h = ecudc_asUInt32(udc);
  
  // write datatype identifier
  ecstream_appendc(stream, ENTC_BINSTYPE_UINT);
  ecstream_appendd(stream, (const char*)&h, 4);    
}

//-----------------------------------------------------------------------------------------------------------

void ecbins_writeInt64 (EcStream stream, const EcUdc udc)
{
  int64_t h = ecudc_asInt64(udc);
  
  // write datatype identifier
  ecstream_appendc(stream, ENTC_BINSTYPE_LONG);
  ecstream_appendd(stream, (const char*)&h, 8);  
}  

//-----------------------------------------------------------------------------------------------------------

void ecbins_writeUInt64 (EcStream stream, const EcUdc udc)
{
  uint64_t h = ecudc_asUInt64(udc);
  
  // write datatype identifier
  ecstream_appendc(stream, ENTC_BINSTYPE_ULONG);
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
    case ENTC_UDC_UBYTE:
    {
      ecbins_writeUByte (stream, udc);
    }
    break;
    case ENTC_UDC_INT16:
    {
      ecbins_writeInt16 (stream, udc);
    }
    break;
    case ENTC_UDC_UINT16:
    {
      ecbins_writeUInt16 (stream, udc);
    }
    break;
    case ENTC_UDC_INT32:
    {
      ecbins_writeInt32 (stream, udc);
    }
    break;
    case ENTC_UDC_UINT32:
    {
      ecbins_writeUInt32 (stream, udc);
    }
    break;
    case ENTC_UDC_INT64:
    {
      ecbins_writeInt64 (stream, udc);
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

EcBuffer ecbins_write (const EcUdc udc, const EcString begining)
{
  EcStream stream = ecstream_new();
  
  if (isAssigned (begining))
  {
    ecstream_append (stream, begining);
  }
  
  ecbins_writeElement (stream, udc);
  
  return ecstream_trans(&stream);
}

//-----------------------------------------------------------------------------------------------------------
