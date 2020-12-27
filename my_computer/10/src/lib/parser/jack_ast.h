#pragma once

#include <string>

#include "tokenizer/jack_tokenizer.h"

typedef enum class AstNodeType_s {
  N_UNDEFINED,
  N_CLASS,
  N_EXPRESSION,
  N_OP,
  N_INTEGER_CONSTANT,
  N_STRING_CONSTANT,
  N_KEYWORD_CONSTANT,
  N_VARIABLE,
  N_1D_ARRAY,  // <var-name>[expr]
  N_TERM,
} AstNodeType_t;

class AstNode : public std::enable_shared_from_this<AstNode> {
public:
  // Constructor for root node
  AstNode(AstNodeType_t ty) : type(ty)
  {
    child_nodes = std::make_unique<std::vector<std::shared_ptr<AstNode>>>();
  }

  // Constructor for child nodes
  // AstNode(std::shared_ptr<AstNode> p, AstNodeType_t ty) : type(ty), parent(p)
  // {}
  AstNode(std::shared_ptr<AstNode> p, AstNodeType_t ty, JackToken& tok)
      : type(ty), token(tok), parent(p)
  {
    child_nodes = std::make_unique<std::vector<std::shared_ptr<AstNode>>>();
  }

  AstNode() = delete;
  AstNode(const AstNode&) = delete;
  AstNode& operator=(const AstNode&) = delete;

  // std::shared_ptr<AstNode> get_shared_ptr() { return shared_from_this(); }

  std::shared_ptr<AstNode> create_child(AstNodeType_t, JackToken&);
  std::vector<std::shared_ptr<AstNode>> get_child_nodes()
  {
    return *child_nodes;
  }

  size_t num_child_nodes() const { return (*child_nodes).size(); }

  // AstNode(std::unique_ptr<AstNode> parent) : parent(parent) {}
  // AstNode(const AstNode& n) : token(n.token), type(n.type){};
  // void operator=(const AstNode&);

  // AstNode(JackToken tok, AstNodeType_t ty) : token(tok), type(ty) {}

  AstNodeType_t type{AstNodeType_t::N_UNDEFINED};
  JackToken token;

private:
  std::shared_ptr<AstNode> parent{nullptr};
  std::unique_ptr<std::vector<std::shared_ptr<AstNode>>> child_nodes{};
};

class JackAst {
public:
  JackAst() = delete;
  JackAst(const JackAst&) = delete;
  JackAst& operator=(const JackAst&) = delete;

  JackAst(AstNodeType_t, std::unique_ptr<std::vector<JackToken>>&);

  // int add_child(std::unique_ptr<AstNode>&, std::unique_ptr<AstNode>);

  std::shared_ptr<AstNode>& get_root() { return root; }

  std::unique_ptr<AstNode> parse_term(size_t start_index = 0);
  std::unique_ptr<AstNode> parse_expression(size_t start_index = 0);

  // std::unique_ptr<std::vector<std::unique_ptr<AstNode>>> children;

private:
  const std::unique_ptr<std::vector<JackToken>>& tokens;

  // int parse_cursor{0};

  std::shared_ptr<AstNode> root{nullptr};
};
