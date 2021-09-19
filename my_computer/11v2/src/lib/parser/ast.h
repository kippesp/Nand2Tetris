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
  N_PARAMETER_LIST,
  N_RETURN_STATEMENT,
  N_STATEMENT_BLOCK,
  //
  N_FUNCTION_DECL,
  N_METHOD_DECL,
  N_CONSTRUCTOR_DECL,
  //
  N_SUBROUTINE_BODY,
  N_SUBROUTINE_DECL_RETURN_TYPE,
  N_SUBROUTINE_DESCR,
  //
  N_KEYWORD_CONSTANT,
  N_TRUE_KEYWORD,
  N_FALSE_KEYWORD,
  N_NULL_KEYWORD,
  N_THIS_KEYWORD,
  N_LET_STATEMENT,
  N_SCALAR_VAR,
  N_TERM,
  N_VARIABLE_NAME,

  N_CLASSVAR_DECL_BLOCK,
  N_PARAMETER_DECL,

  // N_ARRAY_BINDING,
  // N_ARRAY_VAR,
  // N_BINARY_OP,
  N_CLASSVAR_STATIC_DECL,
  N_CLASSVAR_FIELD_DECL,
  N_VARIABLE_INTEGER_TYPE,
  N_VARIABLE_BOOLEAN_TYPE,
  N_VARIABLE_CHARACTER_TYPE,
  N_VARIABLE_CLASS_TYPE,
  // N_DELIMITER,
  N_DO_STATEMENT,
  // N_EXPRESSION_LIST,
  // N_IF_STATEMENT,
  // N_KEYWORD,
  // N_OP,
  // N_RETURN_TYPE,
  // N_STRING_CONSTANT,
  // N_SUBROUTINE_CALL,
  // N_SUBROUTINE_CALL_SITE_BINDING,
  N_LOCAL_VAR_DECL_BLOCK,
  N_LOCAL_VAR_DECL,
  // N_SUBROUTINE_TYPE,
  // N_UNARY_OP,
  // N_UNDEFINED,
  // N_VARIABLE_DECL,
  // N_VARIABLE_LIST,
  // N_VARIABLE_TYPE,
  // N_VAR_DECL_BLOCK,
  // N_VAR_DECL_STATEMENT,
  // N_WHILE_STATEMENT,
};

#if 0
using AstNodeType_t = std::variant<
  //std::monostate,
AstNodeType_t
  >;
#endif

class AstNode;

using AstNodeRef = std::reference_wrapper<AstNode>;
using AstNodeCRef = std::reference_wrapper<const AstNode>;

using AstNodeValue_t = std::variant<std::monostate, std::string, int>;

class AstNode {
public:
  const AstNodeType_t type;
  AstNodeValue_t value;

  AstNode() = delete;
  // AstNode(const AstNode&) = delete;

  AstNode(AstNodeType_t type);

  AstNode(const AstNode& my_class) : type(my_class.type) {}

  AstNode& operator=(const AstNode&) = delete;

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

  // static AstTree parse_ast(const std::vector<const JackToken>);

  AstNodeRef add(const AstNode& node);

private:
  // All nodes of the tree with nodes[0] being the root
  std::vector<std::unique_ptr<AstNode>> nodes;
};

#if 0
class AstNode : public std::enable_shared_from_this<AstNode> {
public:
  AstNode() = delete;
  AstNode(const AstNode&) = delete;
  AstNode& operator=(const AstNode&) = delete;

  virtual ~AstNode() = default;

  static std::string to_string(AstNodeType_t);

  friend std::ostream& operator<<(std::ostream&, const AstNode&);

  const AstNodeType_t type{AstNodeType_t::P_UNDEFINED};

protected:
  AstNode(AstNodeType_t type) : type(type) {}

private:
};

class ParseTreeTerminal;

class ParseTreeNonTerminal : public AstNode {
public:
  ParseTreeNonTerminal() = delete;
  ParseTreeNonTerminal(const ParseTreeNonTerminal&) = delete;
  ParseTreeNonTerminal& operator=(const ParseTreeNonTerminal&) = delete;

  ParseTreeNonTerminal(AstNodeType_t ty)
      : AstNode(ty),
        child_nodes(
            std::make_unique<std::vector<std::shared_ptr<AstNode>>>())
  {
  }

  // Create a non-terminal node
  std::shared_ptr<AstNode> create_child(
      std::shared_ptr<ParseTreeNonTerminal>);

  // Create a terminal node
  std::shared_ptr<AstNode> create_child(AstNodeType_t, JackToken&);
  std::shared_ptr<AstNode> create_child(
      std::shared_ptr<ParseTreeTerminal>);

  size_t num_child_nodes() const { return child_nodes->size(); }

  std::vector<std::shared_ptr<AstNode>> get_child_nodes() const
  {
    return *child_nodes;
  }

private:
  std::unique_ptr<std::vector<std::shared_ptr<AstNode>>> child_nodes{
      nullptr};
};

class ParseTreeTerminal : public AstNode {
public:
  ParseTreeTerminal() = delete;
  ParseTreeTerminal(const ParseTreeTerminal&) = delete;
  ParseTreeTerminal& operator=(const ParseTreeTerminal&) = delete;

  ParseTreeTerminal(AstNodeType_t ty, JackToken& token)
      : AstNode(ty), token(token)
  {
  }

  const JackToken token;

private:
};

class ParseTree {
public:
  ParseTree() = delete;
  ParseTree(const ParseTree&) = delete;
  ParseTree& operator=(const ParseTree&) = delete;

  // Earlier constructor before parsers were developed
  ParseTree(AstNodeType_t type,
            std::unique_ptr<std::vector<JackToken>>& tokens)
      : root(std::make_shared<ParseTreeNonTerminal>(type)),
        tokens(tokens),
        base_filename("%")
  {
  }

  ParseTree(std::unique_ptr<std::vector<JackToken>>& tokens,
            std::string s = "%")
      : tokens(tokens), base_filename(s)
  {
  }

  std::string pprint(std::string_view) const;

  // program structure parsers
  std::shared_ptr<ParseTreeNonTerminal> parse_class();
  std::shared_ptr<ParseTreeNonTerminal> parse_class_variable_decl_block();
  std::shared_ptr<ParseTreeNonTerminal> parse_subroutine_declaration();
  std::shared_ptr<ParseTreeNonTerminal> parse_parameter_list();
  std::shared_ptr<ParseTreeNonTerminal> parse_subroutine_body();
  std::shared_ptr<ParseTreeNonTerminal> parse_variable_decl_block();

  // helper to parse the list of one or more variables in either
  // a var or class declaration
  std::shared_ptr<ParseTreeNonTerminal> parse_variable_decl_list();

  // statement parsers
  std::shared_ptr<ParseTreeNonTerminal> parse_statement();
  std::shared_ptr<ParseTreeNonTerminal> parse_statements();
  std::shared_ptr<ParseTreeNonTerminal> parse_let_statement();
  std::shared_ptr<ParseTreeNonTerminal> parse_if_statement();
  std::shared_ptr<ParseTreeNonTerminal> parse_while_statement();
  std::shared_ptr<ParseTreeNonTerminal> parse_do_statement();
  std::shared_ptr<ParseTreeNonTerminal> parse_return_statement();

  // expression parsers
  std::shared_ptr<ParseTreeNonTerminal> parse_expression();
  std::shared_ptr<ParseTreeNonTerminal> parse_expression_list();
  std::shared_ptr<ParseTreeNonTerminal> parse_term();
  std::shared_ptr<ParseTreeTerminal> parse_op();
  std::shared_ptr<ParseTreeNonTerminal> parse_subroutine_call();

  static std::unique_ptr<ParseTree> build_parse_tree(
      std::unique_ptr<std::vector<JackToken>>&);

  const std::shared_ptr<ParseTreeNonTerminal>& get_root() { return root; }

private:
  const std::shared_ptr<ParseTreeNonTerminal> root{nullptr};
  const std::unique_ptr<std::vector<JackToken>>& tokens;
  size_t parse_cursor{0};
  const std::string base_filename;
};

class ParseException : public std::exception {
public:
  ParseException(std::string, const JackToken&);

  const std::string description;
  const JackToken token;

  virtual const char* what() const noexcept { return description.data(); }
};
#endif
}  // namespace ast
