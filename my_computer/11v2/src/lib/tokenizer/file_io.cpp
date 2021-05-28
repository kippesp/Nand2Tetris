#include "file_io.h"

#include <errno.h>
#include <stdio.h>

#include <cassert>
#include <fstream>

using namespace std;

JackInputFile::JackInputFile(const string& filename)
    : Reader(), filename(filename)
{
}

JackInputFile::~JackInputFile()
{
  if (infile)
  {
    fclose(infile);
    infile = nullptr;
  }
}

int JackInputFile::open()
{
  assert(infile == nullptr);

  infile = fopen(filename.c_str(), "r");

  if (!infile)
  {
    perror("open");
    return 1;
  }

  return 0;
}

JackInputFile::char_type JackInputFile::peek()
{
  char_type c = read();
  ungetc(c, infile);
  return c;
}

JackInputFile::char_type JackInputFile::read()
{
  assert(infile && "open() not called");

  if (!infile) return -1;

  char_type c = fgetc(infile);

  // check for EOF or error
  if (c == EOF)
  {
    return -1;
  }

  return c;
}
