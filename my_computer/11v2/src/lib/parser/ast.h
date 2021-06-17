#pragma once

//#include <memory>
#include <vector>

#include "tokenizer/jack_tokenizer.h"

namespace ast {

typedef enum class AstNodeType_s {
  N_CLASS_DECL,
  N_EXPRESSION,
  N_INTEGER_CONSTANT,
  N_PARAMETER_LIST,
  N_RETURN_STATEMENT,
  N_STATEMENT_BLOCK,
  N_SUBROUTINE_BODY,

  // N_ARRAY_BINDING,
  // N_ARRAY_VAR,
  // N_BINARY_OP,
  // N_CLASSVAR_DECL_BLOCK,
  // N_CLASSVAR_DECL_STATEMENT,
  // N_CLASSVAR_SCOPE,
  // N_CLASS_OR_VAR_NAME,       need a clearer name here...identifier name??
  // N_DELIMITER,
  // N_DO_STATEMENT,
  // N_EXPRESSION_LIST,
  // N_IF_STATEMENT,
  // N_KEYWORD,
  // N_KEYWORD_CONSTANT,
  // N_LET_STATEMENT,
  // N_OP,
  // N_RETURN_TYPE,
  // N_SCALAR_VAR,
  // N_STRING_CONSTANT,
  // N_SUBROUTINE_CALL,
  // N_SUBROUTINE_CALL_SITE_BINDING,
  // N_SUBROUTINE_DECL_BLOCK,
  // N_SUBROUTINE_TYPE,
  // N_TERM,
  // N_UNARY_OP,
  // N_UNDEFINED,
  // N_VARIABLE_DECL,
  // N_VARIABLE_LIST,
  // N_VARIABLE_NAME,
  // N_VARIABLE_TYPE,
  // N_VAR_DECL_BLOCK,
  // N_VAR_DECL_STATEMENT,
  // N_WHILE_STATEMENT,
} AstNodeType_t;

class AstTree;

class AstNode {
public:
  AstNode() = delete;
  AstNode(const AstNode&) = delete;
  AstNode& operator=(const AstNode&) = delete;

private:
  const AstTree* tree {nullptr};
  const AstNode* parent_node {nullptr};
};

class AstTree {
public:
  AstTree() = delete;
  AstTree(const AstTree&) = delete;
  AstTree& operator=(const AstNode&) = delete;

private:
  std::vector<AstNode> nodes;
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
