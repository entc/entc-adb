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

#ifndef ADBLCPP_ADBL_H
#define ADBLCPP_ADBL_H 1

#include <types/ocstring.h>
#include "adbl.h"
#include "adbl_structs.h"
#include "adbl_manager.h"

namespace entc {

  class OcAdblConstraint
  {
    
    friend class OcAdblQuery;
    friend class OcAdblUpdate;
    friend class OcAdblDelete;
    
  public:
    
    OcAdblConstraint()
    : mconstraint(adbl_constraint_new( QUOMADBL_CONSTRAINT_AND ))
    {
    }
    
    ~OcAdblConstraint()
    {
      adbl_constraint_delete(&(this->mconstraint));
    }
    
    void addEqual(const OcString& column, const OcString& value)
    {
      adbl_constraint_addChar(this->mconstraint, column.cstr(), QUOMADBL_CONSTRAINT_EQUAL, value.cstr());
    }
    
  private:
    
    AdblConstraint* mconstraint;
    
  };
  
  class OcAdblAttributes
  {
    
    friend class OcAdblUpdate;
    friend class OcAdblInsert;
    
  public:
    
    OcAdblAttributes()
    : mattrs(adbl_attrs_new())
    {
    }
    
    ~OcAdblAttributes()
    {
      adbl_attrs_delete(&(this->mattrs));
    }
    
    void add(const OcString& column, const OcString& value)
    {
      adbl_attrs_addChar(this->mattrs, column.cstr(), value.cstr() );
    }
    
    bool isEmpty()
    {
      return adbl_attrs_empty(this->mattrs) == TRUE;
    }
    
  private:
    
    AdblAttributes* mattrs;
    
  };
  
  class OcAdblCursor
  {
    
    friend class OcAdblSession;
    
  public:
    
    OcAdblCursor()
    : mcursor(NULL)
    {
    }

    ~OcAdblCursor()
    {
      if(this->mcursor != NULL)
      {
        adbl_dbcursor_release(&(this->mcursor));        
      }
    }
    
    bool isValid()
    {
      return (this->mcursor != NULL);
    }
    
    bool next()
    {
      if(this->mcursor != NULL)
      {
        return adbl_dbcursor_next(this->mcursor) == TRUE;
      }
      else
      {
        return false;
      }
    }
    
    const EcString data()
    {
      return adbl_dbcursor_nextdata(this->mcursor);
    }
    
  private:
    
    AdblCursor* mcursor;
    
  };
  
  class OcAdblQuery
  {
    
    friend class OcAdblSession;
    
  public:
    
    OcAdblQuery()
    : mquery(adbl_query_new())
    {
    }
    
    ~OcAdblQuery()
    {
      adbl_query_delete(&(this->mquery));
    }
    
    void setTable(const OcString& tablename)
    {
      adbl_query_setTable(this->mquery, tablename.cstr());
    }
    
    void setConstraint(OcAdblConstraint& constraint)
    {
      adbl_query_setConstraint(this->mquery, constraint.mconstraint);
    }
    
    void addColumn(const OcString& column, int order_pos = 0)
    {
      if(column.isValid())
      {
        adbl_query_addColumn(this->mquery, column.cstr(), order_pos);    
      }
    }
    
    void setLimit(uint_t maxrows)
    {
      adbl_query_setLimit(this->mquery, maxrows);
    }
    
    void setOffset(uint_t offset)
    {
      adbl_query_setOffset(this->mquery, offset);  
    }
    
  private:
    
    AdblQuery* mquery;
    
  };
  
  class OcAdblUpdate
  {
    
    friend class OcAdblSession;
   
  public:
    
    OcAdblUpdate()
    : mupdate(adbl_update_new())
    {
    }
    
    ~OcAdblUpdate()
    {
      adbl_update_delete(&(this->mupdate));
    }
    
    void setTable(const OcString& tablename)
    {
      adbl_update_setTable(this->mupdate, tablename.cstr());
    }
    
    void setConstraint(OcAdblConstraint& constraint)
    {
      adbl_update_setConstraint(this->mupdate, constraint.mconstraint);
    }
    
    void setAttributes(OcAdblAttributes& attributes)
    {
      adbl_update_setAttributes(this->mupdate, attributes.mattrs);
    }
    
  private:
    
    AdblUpdate* mupdate;
    
  };
  
  class OcAdblInsert
  {
    
    friend class OcAdblSession;
    
  public:
    
    OcAdblInsert()
    : minsert(adbl_insert_new())
    {
    }
    
    ~OcAdblInsert()
    {
      adbl_insert_delete(&(this->minsert));
    }
    
    void setTable(const OcString& tablename)
    {
      adbl_insert_setTable(this->minsert, tablename.cstr());
    }
    
    void setAttributes(OcAdblAttributes& attributes)
    {
      adbl_insert_setAttributes(this->minsert, attributes.mattrs);
    }
    
  private:
    
    AdblInsert* minsert;
    
  };
  
  class OcAdblDelete
  {
    
    friend class OcAdblSession;
    
  public:
    
    OcAdblDelete()
    : mdelete(adbl_delete_new())
    {
    }
    
    ~OcAdblDelete()
    {
      adbl_delete_delete(&(this->mdelete));
    }
    
    void setTable(const OcString& tablename)
    {
      adbl_delete_setTable(this->mdelete, tablename.cstr());
    }
    
    void setConstraint(OcAdblConstraint& constraint)
    {
      adbl_delete_setConstraint(this->mdelete, constraint.mconstraint);
    }
    
  private:
    
    AdblDelete* mdelete;
    
  };
  
  
  class OcAdblSequence
  {
    
    friend class OcAdblSession;
    
  public:
    
    OcAdblSequence()
    : msequence(NULL)
    {
    }
    
    ~OcAdblSequence()
    {
      if(msequence)
      {
        adbl_sequence_release(&(this->msequence));
      }
    }
    
    bool isValid()
    {
      return msequence != NULL;
    }
    
    uint_t next()
    {
      return adbl_sequence_next(this->msequence);
    }
        
  private:
    
    AdblSequence* msequence;
    
  };
  
  
  class OcAdblSession
  {
    
  public:
    
    OcAdblSession(AdblSession session) : msession(session) {}
    
    ~OcAdblSession()
    {
      if (this->msession != NULL) {
        adbl_closeSession(&(this->msession));
      }
    }
    
    bool isValid()
    {
      return (this->msession != NULL);
    }
    
    void begin()
    {
      adbl_dbbegin(this->msession);
    }
    
    void commit()
    {
      adbl_dbcommit(this->msession);
    }
    
    void rollback()
    {
      adbl_dbrollback(this->msession);
    }
    
    bool applyQuery(OcAdblQuery& query, OcAdblCursor& cursor, EcLogger logger)
    {
      if (this->msession)
      {
        AdblSecurity adblsec;
        
        cursor.mcursor = adbl_dbquery(this->msession, query.mquery, &adblsec );
        
        if( adblsec.inicident !=0 )
        {
          eclogger_sec (logger, SL_RED);
          eclogger_log (logger, LL_DEBUG, "ADBL", "a security incident was reported!" );
        }
        
        return cursor.isValid();
      }
      return false;
    }
    
    uint_t applyUpdate(OcAdblUpdate& update, EcLogger logger)
    {
      AdblSecurity adblsec;
      
      int res = adbl_dbupdate(this->msession, update.mupdate, &adblsec);
      
      if( adblsec.inicident !=0 )
      {
        eclogger_sec (logger, SL_RED);
        eclogger_log (logger, LL_DEBUG, "ADBL", "a security incident was reported!" );
      }
      
      return res;      
    }
    
    int applyInsert(OcAdblInsert& insert, EcLogger logger)
    {
      AdblSecurity adblsec;
      
      int res = adbl_dbinsert(this->msession, insert.minsert, &adblsec );
      
      if( adblsec.inicident !=0 )
      {
        eclogger_sec (logger, SL_RED);
        eclogger_log(logger, LL_DEBUG, "ADBL", "a security incident was reported!" );
      }
      
      return res;
    }
    
    int applyDelete(OcAdblDelete& dbdelete, EcLogger logger)
    {
      AdblSecurity adblsec;
      
      int res = adbl_dbdelete(this->msession, dbdelete.mdelete, &adblsec);
      
      if( adblsec.inicident !=0 )
      {
        eclogger_sec (logger, SL_RED);
        eclogger_log(logger, LL_DEBUG, "ADBL", "a security incident was reported!" );
      }
      
      return res;
    }
    
    void sequence(OcAdblSequence& sequence, const OcString& tablename)
    {
      sequence.msequence = adbl_dbsequence_get(this->msession, tablename.cstr());
    }
    
    uint_t getTableSize(const OcString& tablename)
    {
      return adbl_table_size(this->msession, tablename.cstr());      
    }
    
  private:
    
    AdblSession msession;
    
  };


}

#endif
