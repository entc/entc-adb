#include "tests/ecenv.h"
#include "system/ectime.h"

#include <stdio.h>

//=============================================================================

static int __STDCALL test_time1 (void* ptr, TestEnvContext ctx, EcErr err)
{
  EcTime t1;
  EcDate d1;
  EcBuffer buf;
  
  ectime_utc_current_time (&t1);
  
  printf ("T1: %i | %i\n", t1.sec, t1.msec);
    
  ectime_utc_date (&d1);
  
  printf ("UTC Time: %i-%02i-%02i %02i:%02i:%02i.%03i\n", d1.year, d1.month, d1.day, d1.hour, d1.minute, d1.sec, d1.msec);
  
  ectime_local_date (&d1);
  
  printf ("LCL Time: %i-%02i-%02i %02i:%02i:%02i.%03i\n", d1.year, d1.month, d1.day, d1.hour, d1.minute, d1.sec, d1.msec);
  
  ectime_utc_date (&d1);

  ectime_date_utc_to_localtime (&d1);

  printf ("UTC -> LCL Time: %i-%02i-%02i %02i:%02i:%02i.%03i\n", d1.year, d1.month, d1.day, d1.hour, d1.minute, d1.sec, d1.msec);

  buf = ecbuf_create (40);
  
  ectime_toGmtString (buf, &d1);
  
  printf ("GMT %s\n", buf->buffer);

  ectime_toISO8601 (buf, &d1);

  printf ("ISO8601 %s\n", buf->buffer);

  ectime_toAlphaNum (buf, &d1);

  printf ("AlphaNum %s\n", buf->buffer);

  ectime_toString (buf, &d1);

  printf ("String %s\n", buf->buffer);

  ectime_toPrefix (buf, &d1);

  printf ("Prefix %s\n", buf->buffer);

  ectime_toPaddedTimestamp (buf, &d1);
  
  printf ("Padded %s\n", buf->buffer);

  return ENTC_ERR_NONE;
}

//=============================================================================

int main(int argc, char* argv[])
{
  TestEnv te = testenv_create ();
  
  testenv_reg (te, "Test1", NULL, NULL, test_time1);
  
  testenv_run (te);
  
  return testenv_destroy (&te);
}

//---------------------------------------------------------------------------

