#pragma once

#include <string>

namespace jfcl {

class SemanticException : public std::exception {
public:
  SemanticException(std::string s) : description(s) {}
  SemanticException(std::string s1, std::string s2)
      : description(s1 + ": " + s2)
  {
  }

  const std::string description;

  const char* what() const noexcept override;
};

}  // namespace jfcl
