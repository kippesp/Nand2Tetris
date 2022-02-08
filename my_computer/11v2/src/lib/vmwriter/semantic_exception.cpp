#include "semantic_exception.h"

namespace jfcl {

const char* SemanticException::what() const noexcept { return description.data(); }

}  // namespace jfcl
