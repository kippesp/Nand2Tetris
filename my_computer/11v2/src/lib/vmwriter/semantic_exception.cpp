#include "semantic_exception.h"

const char* SemanticException::what() const noexcept { return description.data(); }
