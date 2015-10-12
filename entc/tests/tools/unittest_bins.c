#include "tools/ecbins.h"

#include "tools/ecjson.h"

//-------------------------------------------------------------------------------------

int main (int argc, char *argv[])
{
  EcUdc mainNode = ecudc_create(ENTC_UDC_NODE, NULL);
  
  ecudc_add_asString (mainNode, "col1", "hello world");
  ecudc_add_asString (mainNode, "col2", "good");
  
  ecudc_add_asInt16(mainNode, "c_int16", 42);
  
  EcUdc sideList = ecudc_create(ENTC_UDC_LIST, "list");
  
  ecudc_add_asUByte(sideList, "l_ubyte", 45);
  ecudc_add_asByte(sideList, "l_byte", -45);

  ecudc_add (mainNode, &sideList);
  
  {
    EcString jtext = ecjson_write(mainNode);
    
    printf("(1) : %s\n", jtext);
    
    ecstr_delete(&jtext);
  }
  
  EcBuffer h = ecbins_write (mainNode, NULL);
  
  EcUdc copy = ecbins_read (h, NULL);

  {
    EcString jtext = ecjson_write(copy);
    
    printf("(2) : %s\n", jtext);
    
    ecstr_delete(&jtext);
  }
  
  return 0;
}
