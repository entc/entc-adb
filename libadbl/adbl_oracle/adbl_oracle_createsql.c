#include "adbl_oracle_createsql.h"

#include "adbl_structs.h"
#include "quomlist.h"

/*------------------------------------------------------------------------*/

void
qdbl_oracle_createsql_columns( struct QCStream* statement, struct QCList* columns, const char* table )
{
  struct QCListNode* node = qclist_first(columns);
  
  if( node != qclist_end(columns) )
  /* more than one entry */
  {
    qcstream_append( statement, table );
    qcstream_append( statement, "." );
    qcstream_append( statement, qclist_data(node) );
    
    node = qclist_next(node);
    
    for(; node != qclist_end(columns); node = qclist_next(node) )
    {
      qcstream_append( statement, ", " );
      qcstream_append( statement, table );
      qcstream_append( statement, "." );
      qcstream_append( statement, qclist_data(node) );
    }
  }
  else
  {
    qcstream_append( statement, "*" );
  }
}

/*------------------------------------------------------------------------*/

void
qdbl_oracle_createsql_constraint_element( struct QCStream* statement, struct QDBLConstraintElement* element )
{
  if( element->type == QUOMADBL_CONSTRAINT_EQUAL )
  {
    if( element->constraint )
    {
      
    }
    else
    {
      qcstream_append( statement, element->column );
      qcstream_append( statement, " = \'" );
      qcstream_append( statement, element->value );
      qcstream_append( statement, "\'" );
    }    
  }
}

/*------------------------------------------------------------------------*/

void
qdbl_oracle_createsql_constraint_node( struct QCStream* statement, struct QDBLConstraint* constraint )
{
  struct QCListNode* node = qclist_first(constraint->list);
  
  if( node != qclist_end(constraint->list) )
  {
    qdbl_oracle_createsql_constraint_element( statement, qclist_data(node) );
    
    node = qclist_next(node);
    
    for(; node != qclist_end(constraint->list); node = qclist_next(node) )
    {
      if( constraint->type == QUOMADBL_CONSTRAINT_AND )
        qcstream_append( statement, " AND " );
      
      qdbl_oracle_createsql_constraint_element( statement, qclist_data(node) );
    }
  }
}

/*------------------------------------------------------------------------*/

void
qdbl_oracle_createsql_constraint( struct QCStream* statement, struct QDBLConstraint* constraint )
{
  struct QCListNode* node = qclist_first(constraint->list);
  
  if( node != qclist_end(constraint->list) )
  {
    qcstream_append( statement, " WHERE " );
    
    qdbl_oracle_createsql_constraint_node( statement, constraint );
  }  
}

/*------------------------------------------------------------------------*/

void
qdbl_oracle_createsql_insert(struct QCStream* statement, struct QDBLAttributes* attrs )
{
  struct QCMapCharNode* node = qcmapchar_first(attrs->columns);
  
  if( node != qcmapchar_end(attrs->columns) )
  {
    struct QCStream* cols = qcstream_new();
    struct QCStream* values = qcstream_new();
    
    qcstream_append( cols, qcmapchar_key(node) );
    
    qcstream_append( values, "\'" );
    qcstream_append( values, qcmapchar_data(node) );
    qcstream_append( values, "\'" );      
    
    node = qcmapchar_next(node);
    
    for(; node != qcmapchar_end(attrs->columns); node = qcmapchar_next(node) )
    {
      qcstream_append( cols, ", " );
      qcstream_append( cols, qcmapchar_key(node) );
      
      qcstream_append( values, ", \'" );
      qcstream_append( values, qcmapchar_data(node) );
      qcstream_append( values, "\'" );      
    }
    qcstream_append( statement, " (" );
    qcstream_append( statement, qcstream_buffer( cols ) );
    qcstream_append( statement, ") VALUES (" );
    qcstream_append( statement, qcstream_buffer( values ) );
    qcstream_append( statement, ")" );
    
    qcstream_delete( cols );
    qcstream_delete( values );
  }
  else
  {
    qcstream_append( statement, " VALUES( NULL )" );
  }  
}

/*------------------------------------------------------------------------*/

int
qdbl_oracle_createsql_update( struct QCStream* statement, struct QDBLAttributes* attrs )
{
  struct QCMapCharNode* node = qcmapchar_first(attrs->columns);
  
  if( node != qcmapchar_end(attrs->columns) )
  {
    qcstream_append( statement, qcmapchar_key(node) );
    qcstream_append( statement, " = \'" );
    qcstream_append( statement, qcmapchar_data(node) );
    qcstream_append( statement, "\'" );
    
    node = qcmapchar_next(node);
    
    for(; node != qcmapchar_end(attrs->columns); node = qcmapchar_next(node) )
    {
      qcstream_append( statement, ", " );
      qcstream_append( statement, qcmapchar_key(node) );
      qcstream_append( statement, " = \'" );
      qcstream_append( statement, qcmapchar_data(node) );
      qcstream_append( statement, "\'" );
    }
    
    return TRUE;
  }
  return FALSE;  
}

/*------------------------------------------------------------------------*/

