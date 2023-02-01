#pragma once

#include "parser/parser.h"
#include "tokenizer/jack_tokenizer.h"
#include "util/text_reader.h"
#include "util/textfile_reader.h"

#include "catch.hpp"

#include <string>
#include <vector>

using Expected_t = std::vector<const char*>;

// like Python '\n'.join(vec_of_strings)
std::string expected_string(const Expected_t&);
