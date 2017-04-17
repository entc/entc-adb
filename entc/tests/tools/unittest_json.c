#include "tools/ecmime.h"

// utils includes
#include "utils/ecmessages.h"
#include "tools/ecjson.h"

// types includes
#include "types/ecbuffer.h"

//-------------------------------------------------------------------------------------------

int main (int argc, char *argv[])
{
  ecmessages_initialize ();

  const char* test1 = "{\
  \"path\": \"files\",\
  \"filename\": \"6AC881E5-C271-4F7F-B2F4-1BABCDC378BF\",\
  \"content\": [\
              [{\
               \"left\": 2.89532293986637,\
               \"top\": 7.559055118110236,\
               \"width\": 67.26057906458797,\
               \"height\": 17.79527559055118,\
               \"party\": {}\
               }]\
              ],\
  \"parties\": {\
    \"5a8ac34e-8bae-4bda-8095-06aaf2bd50bc\": {\
      \"key\": \"5a8ac34e-8bae-4bda-8095-06aaf2bd50bc\",\
      \"userid\": 1,\
      \"name\": \"alex\"\
    }\
  }\
}";
  
  EcUdc data = ecjson_read (test1, NULL);
  
  if (!data)
  {
    eclogger_fmt (LL_ERROR, "TEST", "data", "can't evaluate json from buffer");
  }
  
  EcString text = ecjson_write(data);
  
  eclogger_fmt (LL_INFO, "TEST", "data", text);
  
  ecstr_delete(&text);
  ecudc_destroy(EC_ALLOC, &data);
  
  ecmessages_deinitialize();
  
  return 0;
}

//-------------------------------------------------------------------------------------------
