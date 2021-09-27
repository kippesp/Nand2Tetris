#pragma once

#include <signal.h>

#include <string>
#include <vector>

#include "catch.hpp"
#include "parser/parser.h"
#include "parser/pparser.h"
#include "tokenizer/jack_tokenizer.h"
#include "util/text_reader.h"
#include "util/textfile_reader.h"

using Expected_t = std::vector<const char*>;

// like Python '\n'.join(vec_of_strings)
std::string expected_string(const Expected_t&);
