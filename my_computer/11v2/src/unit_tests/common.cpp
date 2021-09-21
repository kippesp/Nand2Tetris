#include "common.h"

std::string expected_string(const Expected_t& strings)
{
  std::stringstream ss;

  for (const char* s : strings)
  {
    ss << s << "\n";
  }

  std::string rvalue = ss.str();
  rvalue.pop_back();

  return rvalue;
}
