#include "system/ecexec.h"

int main (int argc, char *argv[])
{
  EcLogger logger = eclogger_new(0);
  
#if defined _WIN64 || defined _WIN32
  EcExec exec = ecexec_new("test.bat");
#else
  EcExec exec = ecexec_new("test.sh");
#endif  
  ecexec_addParameter(exec, "test.txt");

  ecexec_run(exec);
  
  eclogger_logformat(logger, LL_DEBUG, "TEST", "got: '%s'", ecexec_stdout(exec));
  
  ecexec_delete(&exec);
  
  eclogger_del (&logger);

  return 0;
}
