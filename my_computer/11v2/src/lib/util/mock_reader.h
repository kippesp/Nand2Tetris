#pragma once

#include "util/text_reader.h"

// probably delete this file

#if 0
namespace test {

class MockReader : public Reader {
public:
  using Reader::char_type;

  char buffer[512] = {0};

  MockReader() = default;
  virtual ~MockReader() = default;

  virtual char_type peek() override { return buffer[index]; }
  virtual char_type read() override { return buffer[index++]; }

private:
  size_t index{0};
};

}  // namespace test
#endif
