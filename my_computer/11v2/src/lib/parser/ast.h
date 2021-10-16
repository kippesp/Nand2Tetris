#pragma once

//#include <memory>
#include <variant>
#include <vector>

#include <iostream>

#include <signal.h>

#include "tokenizer/jack_tokenizer.h"

namespace ast {

using AstNodeType_t = enum class AstNodeType_s {
  N_UNDEFINED,
  N_CLASS_DECL,
  N_EXPRESSION,
  N_INTEGER_CONSTANT,
  N_RETURN_STATEMENT,
  N_STATEMENT_BLOCK,
  //
  N_FUNCTION_DECL,
  N_METHOD_DECL,
  N_CONSTRUCTOR_DECL,
  //
  N_SUBROUTINE_BODY,
  N_SUBROUTINE_DESCR,
  N_RETURN_TYPE,
  N_INPUT_PARAMETERS,
  //
  N_KEYWORD_CONSTANT,
  N_TRUE_KEYWORD,
  N_FALSE_KEYWORD,
  N_NULL_KEYWORD,
  N_THIS_KEYWORD,
  N_LET_STATEMENT,
  // N_TERM,

  N_CLASS_VARIABLES,
  N_LOCAL_VARIABLES,
  N_VARIABLE_TYPE,
  N_VARIABLE_DECL,

  N_SUBSCRIPTED_VARIABLE_NAME,
  N_VARIABLE_NAME,

  N_OP_MULTIPLY,
  N_OP_DIVIDE,
  N_OP_ADD,
  N_OP_SUBTRACT,
  N_OP_LOGICAL_EQUALS,
  N_OP_LOGICAL_GT,
  N_OP_LOGICAL_LT,
  N_OP_LOGICAL_AND,
  N_OP_LOGICAL_OR,

  N_PREFIX_OP_NEGATIVE,
  N_PREFIX_OP_LOGICAL_NOT,

  N_SUBROUTINE_CALL,
  N_SUBROUTINE_NAME,
  N_LOCAL_CALL_SITE,
  N_GLOBAL_CALL_SITE,
  N_GLOBAL_BIND_NAME,

  // N_ARRAY_BINDING,
  // N_ARRAY_VAR,
  // N_BINARY_OP,
  N_CLASS_VARIABLE_SCOPE,
  // N_DELIMITER,
  N_DO_STATEMENT,
  N_WHILE_STATEMENT,
  // N_EXPRESSION_LIST,
  N_IF_STATEMENT,
  // N_KEYWORD,
  // N_OP,
  // N_RETURN_TYPE,
  N_STRING_CONSTANT,
  // N_SUBROUTINE_TYPE,
  // N_UNARY_OP,
  // N_UNDEFINED,
  // N_VARIABLE_LIST,
  // N_VARIABLE_TYPE,
  // N_VAR_DECL_BLOCK,
  // N_VAR_DECL_STATEMENT,
};

// TODO: Better design would be to have AstNode data as a struct with no methods
// then have class that uses it.

class AstNode;

using AstNodeRef = std::reference_wrapper<AstNode>;
using AstNodeCRef = std::reference_wrapper<const AstNode>;

using AstNodeValue_t = std::variant<std::monostate, std::string, int>;

class AstNode {
public:
  const AstNodeType_t type;
  AstNodeValue_t value;

  AstNode() = delete;
  AstNode& operator=(const AstNode&) = delete;

  AstNode(AstNodeType_t type);

  AstNode(const AstNode& my_class) : type(my_class.type) {}

  bool operator==(const AstNode&) const;

  friend std::ostream& operator<<(std::ostream& os, const AstNode& rhs);

  std::string as_s_expression(const std::string& = "");

  AstNodeRef add_child(AstNodeRef);

  static std::string to_string(AstNodeType_t);

  // console printer for s-expression tree
  void dump();

  // access the node's children
  std::vector<AstNodeCRef> get_child_nodes() const;

private:
  std::vector<AstNodeRef> child_nodes;
};

class AstTree {
public:
  AstTree() { nodes.reserve(100); }
  AstTree(const AstTree&) = delete;
  AstTree& operator=(const AstNode&) = delete;

  AstNodeRef add(const AstNode& node);

private:
  // All nodes of the tree with nodes[0] being the root
  std::vector<std::unique_ptr<AstNode>> nodes;
};

}  // namespace ast
