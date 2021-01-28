#pragma once

#include <memory>
#include <vector>

#include "tokenizer/jack_tokenizer.h"

/*
clang-format off

Grammar from Figure 10.5

Program Structure:

<class>            ::= "class" <class-name> "{" {<class-var-decl>}*
                       {<subroutine-decl>}* "}"
<class-var-decl>   ::= ("static" | "field") <type> <var-name> {"," <var-name>}* ";"
<subroutine-decl>  ::= ("constructor" | "function" | "method")
                       ("void" | <type>) <subroutine-name>
                       "(" <parameter-list> ")" <subroutine-body>
<parameter-list>   ::= {(<type> <var-name>) {"," <type> <var-name>}*}?
<subroutine-body>  ::= "{" {<var-decl>}* <statements> "}"
<var-decl>         ::= "var" <type> <var-name> {"," <var-name>}* ";"
<type>             ::= "int" | "char" | "boolean" | <class-name>
<class-name>       ::= <identifier>
<subroutine-name>  ::= <identifier>
<var-name>         ::= <identifier>

Statements:

<statements>       ::= {<statement>}*
<statement>        ::= <let-statement> | <if-statement> | <while-statement> |
                       <do-statement> | <return-statement>
<let-statement>    ::= "let" <var-name> {"[" <expression> "]"}? "=" <expression> ";"
<if-statement>     ::= "if" "(" <expression ")" "{" <statements> "}"
                       {"else" "{" <statements> "}"}?
<while-statement>  ::= "while" "(" <expression> ")" "{" <statements> "}"
<do-statement>     ::= "do" <subroutine-call> ";"
<return-statement> ::= "return" {<expression>}? ";"

Expressions:

<expression>       ::= <term> {<op> <term>}*
<term>             ::= <integer-constant> | <string-constant> | <keyword-constant> |
                       <var-name> | <array-var> | <subroutine-call> |
                       "(" <expression> ")" | <unary-op> <term>
<subroutine-call>  ::= <call-site> "(" <expression-list> ")"
<call-site>        ::= <subroutine-name> |
                       (<class-name> | <var-name>) "." <subroutine-name>
<array-var>        ::= <var-name> "[" <expression> "]" 
<expression-list>  ::= {<expression {"," <expression>}*}?
<op>               ::= "+" | "-" | "*" | "/" | "&" | "|" | "<" | ">" | "="
<unary-op>         ::= "-" | "~"
<keyword-constant> ::= "true" | "false" | "null" | "this"

clang-format on
*/

typedef enum class ParseTreeNodeType_s {
  P_UNDEFINED,
  P_ARRAY_VAR,
  P_CLASS_DECL_BLOCK,
  P_CLASS_NAME,
  P_CLASS_OR_VAR_NAME,
  P_CLASSVAR_DECL_BLOCK,
  P_CLASSVAR_DECL_STATEMENT,
  P_CLASSVAR_SCOPE,
  P_DELIMITER,
  P_DO_STATEMENT,
  P_EXPRESSION,
  P_EXPRESSION_LIST,
  P_IF_STATEMENT,
  P_INTEGER_CONSTANT,
  P_KEYWORD,
  P_KEYWORD_CONSTANT,
  P_LET_STATEMENT,
  P_OP,
  P_PARAMETER_LIST,
  P_RETURN_STATEMENT,
  P_RETURN_TYPE,
  P_SCALAR_VAR,
  P_STATEMENT_LIST,
  P_STRING_CONSTANT,
  P_SUBROUTINE_BODY,
  P_SUBROUTINE_CALL,
  P_SUBROUTINE_CALL_SITE_BINDING,
  P_SUBROUTINE_DECL_BLOCK,
  P_SUBROUTINE_NAME,
  P_SUBROUTINE_TYPE,
  P_TERM,
  P_UNARY_OP,
  P_VAR_DECL_BLOCK,
  P_VAR_DECL_STATEMENT,
  P_VARIABLE_DECL,
  P_VARIABLE_LIST,
  P_VARIABLE_NAME,
  P_VARIABLE_TYPE,
  P_WHILE_STATEMENT,
} ParseTreeNodeType_t;

class ParseTreeNode : public std::enable_shared_from_this<ParseTreeNode> {
public:
  ParseTreeNode() = delete;
  ParseTreeNode(const ParseTreeNode&) = delete;
  ParseTreeNode& operator=(const ParseTreeNode&) = delete;

  virtual ~ParseTreeNode() = default;

  static std::string to_string(ParseTreeNodeType_t);

  friend std::ostream& operator<<(std::ostream&, const ParseTreeNode&);

  const ParseTreeNodeType_t type{ParseTreeNodeType_t::P_UNDEFINED};

protected:
  ParseTreeNode(ParseTreeNodeType_t type) : type(type) {}

private:
};

class ParseTreeTerminal;

class ParseTreeNonTerminal : public ParseTreeNode {
public:
  ParseTreeNonTerminal() = delete;
  ParseTreeNonTerminal(const ParseTreeNonTerminal&) = delete;
  ParseTreeNonTerminal& operator=(const ParseTreeNonTerminal&) = delete;

  ParseTreeNonTerminal(ParseTreeNodeType_t ty)
      : ParseTreeNode(ty),
        child_nodes(
            std::make_unique<std::vector<std::shared_ptr<ParseTreeNode>>>())
  {
  }

  // Create a non-terminal node
  std::shared_ptr<ParseTreeNode> create_child(
      std::shared_ptr<ParseTreeNonTerminal>);

  // Create a terminal node
  std::shared_ptr<ParseTreeNode> create_child(ParseTreeNodeType_t, JackToken&);
  std::shared_ptr<ParseTreeNode> create_child(
      std::shared_ptr<ParseTreeTerminal>);

  size_t num_child_nodes() const { return child_nodes->size(); }

  std::vector<std::shared_ptr<ParseTreeNode>> get_child_nodes() const
  {
    return *child_nodes;
  }

private:
  std::unique_ptr<std::vector<std::shared_ptr<ParseTreeNode>>> child_nodes{
      nullptr};
};

class ParseTreeTerminal : public ParseTreeNode {
public:
  ParseTreeTerminal() = delete;
  ParseTreeTerminal(const ParseTreeTerminal&) = delete;
  ParseTreeTerminal& operator=(const ParseTreeTerminal&) = delete;

  ParseTreeTerminal(ParseTreeNodeType_t ty, JackToken& token)
      : ParseTreeNode(ty), token(token)
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
  ParseTree(ParseTreeNodeType_t type,
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
