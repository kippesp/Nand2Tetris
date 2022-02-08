#pragma once

#include "text_reader.h"

namespace jfcl {

class TextFileReader : public TextReader {
public:
  TextFileReader(const char*);
};

}  // namespace jfcl
