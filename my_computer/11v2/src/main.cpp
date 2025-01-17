#include "jfcl.h"

int main(int argc, const char* argv[])
{
  // check for funny business
  if ((argc == 0) || (argv == nullptr) || (argv[0] == nullptr))
  {
    // no error for you
    return 0;
  }

  return jfcl_main(argc, argv);
}
