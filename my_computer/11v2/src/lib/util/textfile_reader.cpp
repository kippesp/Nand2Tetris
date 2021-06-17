#include "textfile_reader.h"

#include <cstdio>
#include <sstream>

#if 0
// from: https://stackoverflow.com/questions/116038/how-do-i-read-an-entire-file-into-a-stdstring-in-c
auto read_file(std::string_view path) -> std::string
{
  constexpr auto read_size = std::size_t{4096};
  auto stream = std::ifstream{path.data()};
  stream.exceptions(std::ios_base::badbit);
  auto out = std::string{};
  auto buf = std::string(read_size, '\0');
  while (stream.read(&buf[0], read_size))
  {
    out.append(buf, 0, stream.gcount());
  }
  out.append(buf, 0, stream.gcount());
  return out;
}
#endif

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
