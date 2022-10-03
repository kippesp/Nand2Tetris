#include "textfile_reader.h"

#include <cstdio>
#include <sstream>

#if defined _MSC_VER
#pragma warning(disable : 4996)
#endif

namespace jfcl {

TextFileReader::TextFileReader(const char* filename)
{
  FILE* fd = fopen(filename, "r");
  std::stringstream ss;

  int ch;
  while ((ch = fgetc(fd)) != EOF)
  {
    ss << static_cast<char>(ch);
  }

  fclose(fd);

  init_buffer(ss.str());
}

}  // namespace jfcl
