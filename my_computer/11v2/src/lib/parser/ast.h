#pragma once
#include "tokenizer/jack_tokenizer.h"

#include <variant>
#include <vector>

#include <signal.h>

namespace ast {

using AstNodeType_t = enum class AstNodeType_s {
  N_UNDEFINED,

  N_CLASS_DECL,
  N_INTEGER_CONSTANT,
  N_RETURN_STATEMENT,
  N_STATEMENT_BLOCK,

  N_FUNCTION_DECL,
  N_METHOD_DECL,
  N_CONSTRUCTOR_DECL,

  N_SUBROUTINE_BODY,
  N_SUBROUTINE_DESCR,
  N_RETURN_TYPE,
  N_INPUT_PARAMETERS,
  N_CALL_ARGUMENTS,

  N_KEYWORD_CONSTANT,
  N_STRING_CONSTANT,

  N_TRUE_KEYWORD,
  N_FALSE_KEYWORD,
  N_NULL_KEYWORD,
  N_THIS_KEYWORD,
  N_LET_STATEMENT,
  N_DO_STATEMENT,
  N_WHILE_STATEMENT,
  N_IF_STATEMENT,

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

  N_OP_PREFIX_NEG,
  N_OP_PREFIX_LOGICAL_NOT,

  N_SUBROUTINE_CALL,
  N_SUBROUTINE_NAME,
  N_LOCAL_CALL_SITE,
  N_GLOBAL_CALL_SITE,
  N_GLOBAL_BIND_NAME,

  N_CLASS_VARIABLE_SCOPE,

  N_METADATA,
  N_METADATA_KEY,
  N_METADATA_VALUE,
};

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

  std::string as_s_expression(const std::string& = "") const;

  AstNodeRef add_child(AstNodeRef);

  static std::string to_string(AstNodeType_t);

  // console printer for s-expression tree
  void dump() const;

  // access the node's children
  std::vector<AstNodeCRef> get_child_nodes() const;

  int num_child_nodes() const { return child_nodes.size(); }

private:
  std::vector<AstNodeRef> child_nodes;
};

class AstTree {
public:
  AstTree()
      : EmptyNode(
            std::make_unique<ast::AstNode>(ast::AstNodeType_t::N_UNDEFINED)),
        EmptyNodeRef(*EmptyNode)
  {
    nodes.reserve(100);
  }
  AstTree(const AstTree&) = delete;
  AstTree& operator=(const AstNode&) = delete;

  AstNodeRef add(const AstNode& node);

  const AstNodeRef& get_empty_node_ref() const { return EmptyNodeRef; }

  const AstNodeCRef get_root() const { return *(nodes[0]); };

  AstNodeCRef find_child_node(ast::AstNodeCRef, ast::AstNodeType_t) const;

private:
  // All nodes of the tree with nodes[0] being the root
  std::vector<std::unique_ptr<AstNode>> nodes;

  // convention to represent an empty leaf
  const std::unique_ptr<AstNode> EmptyNode;
  const AstNodeRef EmptyNodeRef;
};

}  // namespace ast
