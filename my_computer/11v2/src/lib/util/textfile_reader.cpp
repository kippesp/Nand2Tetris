#include "textfile_reader.h"

#include <cstdio>
#include <sstream>

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

  ss << '\0';

  init_buffer(ss.str().data());
}
