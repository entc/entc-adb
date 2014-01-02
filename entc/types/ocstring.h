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

#ifndef ENTCPP_TYPES_OSTRING_H
#define ENTCPP_TYPES_OSTRING_H 1

#include <system/macros.h>

#include <types/ecstring.h>
#include <tools/ecxmlstream.h>
#include <cstring>

namespace entc {

  
  class __LIB_EXPORT OcString
  {
    
  public:
    
    OcString()
    : _s( ecstr_init() )
    {
    }
    
    OcString(const EcString c)
    : _s( ecstr_copy(c) )
    {
    }
    
    OcString(const OcString& rhs)
    : _s( ecstr_copy( rhs._s ) )
    {
    }
    
    ~OcString()
    {
      clear();  
    }
    
    const EcString cstr() const
    {
      return _s;  
    }
    
    void clear()
    {
      ecstr_delete( &_s );
      
      _s = 0;
    }
    
    bool isValid() const
    {
      return ecstr_valid(_s) == TRUE;  
    }
    
    bool isEmpty() const
    {
      if( !isValid() )
      {
        return true;
      }
      // check first character
      return _s[0] == 0;
    }
    
    bool hasLeading( char c ) const
    {
      return _s[0] == c;
    }
    
    bool hasLeading( char c1, char c2, char c3 ) const
    {
      if( strlen(_s) > 3 )
      {
        return (_s[0] == c1) && (_s[1] == c2) && (_s[2] == c3); 
      }
      
      return false;
    }
    
    bool hasLeading( const EcString s ) const
    {
      uint_t slen = strlen(s);
      
      if( strlen(_s) > slen )
      {
        
      }
      
      return false;
    }
    
    void trimLeft(uint_t n)
    {
      ecstr_replace(&_s, _s + n);
    }
    
    OcString& operator+(const OcString& rhs)
    {
      ecstr_replaceTO(&_s, ecstr_cat2(_s, rhs._s));
      
      return *this;
    }
    
    bool operator==(const OcString& rhs) const
    {
      return ecstr_equal(_s, rhs._s) == TRUE;
    }
    
    bool operator==(const EcString src) const
    {
      return ecstr_equal(_s, src) == TRUE;
    }
    
    bool operator!=(const OcString& rhs) const
    {
      return !ecstr_equal(_s, rhs._s);
    }
    
    bool operator<(const OcString& rhs) const
    {
      if(!ecstr_valid(rhs._s))
      {
        return true;
      }
      
      if(!ecstr_valid(_s))
      {
        return false;
      }
      
      return strcmp(_s, rhs._s) < 0;  
    }
    
    bool operator!=(const EcString src) const
    {
      return !ecstr_equal(_s, src);
    }
    
    OcString& operator=(const EcString src)
    {
      ecstr_replace(&_s, src);
      
      return *this;
    }
    
    OcString& operator=(const OcString& src)
    {
      assign(src);
      
      return *this;
    }
    
    operator uint_t()
    {
      return atoi(_s);  
    }
    
    /*
    operator const EcString()
    {
      return _s;
    }
    */
    void assignValid(const EcString src)
    {
      if( ecstr_valid(src) )
      {
        ecstr_replace(&_s, src);
      }
    }
    
    void assign(EcString src)
    {
      ecstr_replaceTO(&_s, src);
    }
    
    void assign(const EcString src)
    {
      ecstr_replace(&_s, src);
    }
    
    void assign(const OcString& src)
    {
      if (this != &src)
      {
        ecstr_replace(&_s, src._s);
      }
    }
    
    void assign(const OcString& src, uint_t n)
    {
      ecstr_replaceTO(&_s, ecstr_part(src._s, n));
    }
    
    void assign(uint_t value, uint_t n)
    {
      ecstr_replaceTO(&_s, ecstr_long(value));
    }
    
    void assign(EcXMLStream stream, const EcString node)
    {
      ecxmlstream_parseNodeValue( stream, &_s, node);
    }
    
    const OcString& append(char c, const OcString& rhs)
    {
      ecstr_replaceTO(&_s, ecstr_catc(_s, c, rhs._s) );
                      
      return *this;
    }
       
    const OcString& append(const OcString& s)
    {
      ecstr_replaceTO(&_s, ecstr_cat2(_s, s._s));
      
      return *this;
    }
    
    const OcString& append(const EcString value)
    {
      ecstr_replaceTO(&_s, ecstr_cat2(_s, value));
      
      return *this;
    }
    
    const OcString& append(uint_t value, uint_t n)
    {
      OcString h(ecstr_long(value));
      
      return this->append(h);
    }
    
    
    static OcString cat2(const OcString& s1, const EcString s2)
    {
      return OcString( ecstr_cat2(s1._s, s2) );
    }
    
    static OcString cat3(const EcString s1, const EcString s2, const EcString s3)
    {
      return OcString( ecstr_cat3(s1, s2, s3) );
    }
                          
    static OcString catc(const OcString& s1, char c, const OcString& s2)
    {
      return OcString( ecstr_catc(s1._s, c, s2._s) );
    }
 
    static OcString catc(const OcString& s1, char c, uint_t u, uint_t n)
    {
      OcString h(ecstr_long(u));
      
      return OcString( ecstr_catc(s1._s, c, h._s) );
    }
    
  private:
    
    OcString(EcString c)
    : _s( c )
    {
    }
    
  private:
    
    EcString _s;
    
  };
  
  static const OcString invalid;

}

#endif
