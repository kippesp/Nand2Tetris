#pragma once

#include <cstdint>

class Reader {
public:
  using char_type = int8_t;

  Reader() = default;
  virtual ~Reader() = default;

  virtual char_type peek() = 0;
  virtual char_type read() = 0;

  virtual void savepos() = 0;
  virtual void restorepos() = 0;

  Reader(const Reader&) = delete;
  Reader& operator=(const Reader&) = delete;
  Reader(Reader&&) = delete;
  Reader& operator=(Reader&&) = delete;
};
