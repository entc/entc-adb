#include <utils/eclogger.h>
#include <adbl_manager.h>

#include "../core/adbo_types.h"
#include "../core/adbo_object.h"
#include "../core/adbo_container.h"
#include "../core/adbo_substitute.h"
#include "../core/adbo_schema.h"
 
void test1 (void* ptr, void* obj)
{
  AdboContext context = ptr;
  
  EcString data;

  AdboObject clone;
  
  adbo_object_request (obj, context);
  
  adbo_dump(obj, context->logger);
  
  data = adbo_str (obj); 
  
  eclogger_logformat(context->logger, LL_INFO, "TEST", "original : %s", ecstr_cstring(data));
  
  clone = adbo_object_clone (obj, NULL);
  
  data = adbo_str (clone); 
  
  eclogger_logformat(context->logger, LL_INFO, "TEST", "clone : %s", ecstr_cstring(data));
  
  //eclist_append (objects, clone);  
}

void test2 (void* ptr, void* obj)
{
  AdboContext context = ptr;

  EcString data;
  
  AdboObject obj2 = adbo_at ((AdboObject)obj, "addresses/field1");
  
  data = adbo_str (obj2); 
  
  eclogger_logformat(context->logger, LL_INFO, "TEST", "data : %s", ecstr_cstring(data));
  
  adbo_set (obj2, "Penny Lane");
  
  data = adbo_str (obj2);
  
  eclogger_logformat(context->logger, LL_INFO, "TEST", "data : %s", ecstr_cstring(data));
}

void test3 (void* ptr, void* obj)
{
  AdboContext context = ptr;

  EcString data;
  
  data = adbo_str (obj); 
  
  adbo_object_update (obj, context, TRUE);
  
  eclogger_logformat(context->logger, LL_INFO, "TEST", "data : %s", ecstr_cstring(data)); 
  
  adbo_dump(obj, context->logger);

}

void test4 (void* ptr, void* obj)
{
  AdboContext context = ptr;

  AdboObject obj2;
  
  obj2 = adbo_at ((AdboObject)obj, "addresses");
  adbo_object_delete (obj2, context, TRUE);  

  obj2 = adbo_at ((AdboObject)obj, "test02");
  adbo_object_delete (obj2, context, TRUE);  
}


int main (int argc, char *argv[])
{
  // variables
  AdboContainer objects = adbo_container_new (ADBO_CONTAINER_NODE, NULL);
  struct AdboContext_s context;
  EcXMLStream xmlstream;
  EcEventFiles events;
  
  AdboObject obj2;
  
  EcString currentDir = ecfs_getCurrentDirectory();
  
  context.logger = eclogger_new (0);
  context.adblm = adbl_new (context.logger);
  context.substitutes = adbo_subsmgr_new (currentDir, &context);
  
  events = ece_files_new (context.logger);

  adbl_scan (context.adblm, events, currentDir);
  
  xmlstream = ecxmlstream_openfile("test.xml", context.logger, currentDir);
  /* parse the xml structure */
  while( ecxmlstream_nextNode( xmlstream ) )
  {
    if( ecxmlstream_isBegin( xmlstream, "test" ) )
    {
      adbo_objects_fromXml (objects, &context, xmlstream, "test");
    }
  }
  /* close the file */
  ecxmlstream_close( xmlstream );
  
  adbo_container_iterate (objects, test1, &context);

  adbo_container_iterate (objects, test2, &context);

  adbo_container_iterate (objects, test3, &context);

  adbo_container_iterate (objects, test4, &context);
  
  
  context.schema = adbo_schema_new (&context, "default");
  
  // get direct a object from a database table
  obj2 = adbo_schema_get (context.schema, &context, NULL, "test01");
  
  adbo_dump(obj2, context.logger);
  
  adbo_subsmgr_del (&(context.substitutes));
    
  return 0;
}
