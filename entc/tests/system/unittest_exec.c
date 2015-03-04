#include "system/ecexec.h"

int main (int argc, char *argv[])
{
#if defined _WIN64 || defined _WIN32
  EcExec exec = ecexec_new("test.bat");
#else
  EcExec exec = ecexec_new("test.sh");
#endif  
  ecexec_addParameter(exec, "test.txt");

  ecexec_run(exec);
  
  eclogger_fmt (LL_DEBUG, "TEST", "main", "got: '%s'", ecexec_stdout(exec));
  
  ecexec_delete(&exec);
  
  return 0;
}
