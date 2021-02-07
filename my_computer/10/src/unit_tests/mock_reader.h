#pragma once

#include "tokenizer/reader.h"

namespace test {

class MockReader : public Reader {
public:
  using Reader::char_type;

  char buffer[512] = {0};

  MockReader() = default;
  virtual ~MockReader() = default;

  virtual char_type peek() override { return buffer[index]; }
  virtual char_type read() override { return buffer[index++]; }

  virtual void savepos() override { saved_index = index; }

  virtual void restorepos() override { index = saved_index; }

private:
  size_t index{0};
  size_t saved_index{0};
};

}  // namespace test
