#include "tools/ecbins.h"

#include "tools/ecjson.h"

//-------------------------------------------------------------------------------------

int main (int argc, char *argv[])
{
  EcBuffer h;
  EcUdc sideList, copy;
  EcUdc mainNode = ecudc_create(EC_ALLOC, ENTC_UDC_NODE, NULL);
  
  ecudc_add_asString (EC_ALLOC, mainNode, "col1", "hello world");
  ecudc_add_asString (EC_ALLOC, mainNode, "col2", "good");
  
  ecudc_add_asInt16(EC_ALLOC, mainNode, "c_int16", 42);
  
  sideList = ecudc_create(EC_ALLOC, ENTC_UDC_LIST, "list");
  
  ecudc_add_asUByte(EC_ALLOC, sideList, "l_ubyte", 45);
  ecudc_add_asByte(EC_ALLOC, sideList, "l_byte", -45);

  ecudc_add (mainNode, &sideList);
  
  {
    EcString jtext = ecjson_write(mainNode);
    
    printf("(1) : %s\n", jtext);
    
    ecstr_delete(&jtext);
  }
  
  h = ecbins_write (mainNode, NULL);
  
  copy = ecbins_read (h, NULL);

  {
    EcString jtext = ecjson_write(copy);
    
    printf("(2) : %s\n", jtext);
    
    ecstr_delete(&jtext);
  }
  
  return 0;
}
