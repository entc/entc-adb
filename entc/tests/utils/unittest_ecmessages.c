#include "utils/ecmessages.h"

#include "system/ecthread.h"

#include <stdio.h>

//-------------------------------------------------------------------------------------

int _STDCALL m01 (void* ptr, EcMessageData* dIn, EcMessageData* dOut)
{
  if (isAssigned (dIn))
  {
    printf("m01 with text : %s\n", ecudc_asString(dIn->content));
  }
  else
  {
    printf("m01 called\n");    
  }
  
  if (isAssigned (dOut))
  {
    dOut->type = 0;
    dOut->content = ecudc_create (ENTC_UDC_STRING, NULL);
    ecudc_setS (dOut->content, "Good 01");
  }
  
  return 0;
}

//-------------------------------------------------------------------------------------

int _STDCALL m02 (void* ptr, EcMessageData* dIn, EcMessageData* dOut)
{
  if (isAssigned (dIn))
  {
    printf("m02 with text : %s\n", ecudc_asString(dIn->content));
  }
  else
  {
    printf("m02 called\n");
  }
  
  if (isAssigned (dOut))
  {
    dOut->type = 0;
    dOut->content = ecudc_create (ENTC_UDC_STRING, NULL);
    ecudc_setS (dOut->content, "Good 02");
  }

  printf("send m01 again\n");
  
  ecmessages_send (0x01, 0x01, NULL, NULL);
  
  return 0;
}

//-------------------------------------------------------------------------------------

int _STDCALL c01 (void* ptr, EcMessageData* data, int errorcode)
{
  if (isAssigned (data))
  {
    printf("c01 with text : %s\n", ecudc_asString(data->content));
  }
  else
  {
    printf("c01 called\n");
  }
  
  return 0;
}

//-------------------------------------------------------------------------------------

int _STDCALL th01 (void* ptr)
{
  int i;
  
  for (i = 0; i < 40; i++)
  {  
    EcMessageData d01;
    EcMessagesOutput o1;

    d01.type = 0;
    d01.ref = 0;
    d01.content = ecudc_create (ENTC_UDC_STRING, NULL);
    ecudc_setS (d01.content, "Hello Thread");
    
    o1 = ecmessages_output_create (c01, NULL, 0, 0);
    ecmessages_broadcast (0x01, &d01, o1);
    ecmessages_output_destroy (&o1);

    ecudc_destroy(&(d01.content));    
  }  
  
  return 0;
}

//-------------------------------------------------------------------------------------

int main (int argc, char *argv[])
{
  EcMessageData d01;
  EcMessagesOutput o1;
  EcThread threads [21];
  int i;

  ecmessages_initialize ();
  
  ecmessages_add (0x01, 0x01, m01, NULL);
  ecmessages_add (0x02, 0x01, m02, NULL);
  
  ecmessages_broadcast (0x01, NULL, NULL);
  
  ecmessages_send (0x02, 0x01, NULL, NULL);
  ecmessages_send (0x01, 0x01, NULL, NULL);
  ecmessages_send (0x01, 0x02, NULL, NULL);
    
  d01.type = 0;
  d01.ref = 0;
  d01.content = ecudc_create (ENTC_UDC_STRING, NULL);
  ecudc_setS (d01.content, "Hello World");

  ecmessages_broadcast (0x01, &d01, NULL);
  
  ecudc_destroy(&(d01.content));
  
  o1 = ecmessages_output_create (c01, NULL, 0, 0);
  ecmessages_broadcast (0x01, NULL, o1);
  ecmessages_output_destroy (&o1);
  
  printf("threading !!!!\n");
  
  for (i = 0; i < 20; i++)
  {
    threads [i] = ecthread_new ();
  }
  
  for (i = 0; i < 20; i++)
  {
    ecthread_start(threads [i], th01, NULL);
  }

  for (i = 0; i < 20; i++)
  {
    ecthread_join(threads [i]);
    ecthread_delete(&(threads [i]));
  }
  
  ecmessages_deinitialize ();
  
  return 0;
}
