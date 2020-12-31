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
<type>             ::= "int" | "char" | "boolean" | <class-name>
<subroutine-decl>  ::= ("constructor" | "function" | "method")
                       ("void" | <type>) <subroutine-name>
                       "(" <parameter-list> ")" <subroutine-body>
<parameter-list>   ::= {(<type> <var-name>) {"," <type> <var-name>}*}?
<subroutine-body>  ::= "{" {<var-decl>}* <statements> "}"
<var-decl>         ::= "var" <type> <var-name> {"," <var-name>}* ";"
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
                       <var-name> | <var-name> "[" <expression> "]" |
                       <subroutine-call> | "(" <expression> ")" | <unary-op> <term>
<subroutine-call>  ::= <subroutine-name> "(" <expression-list> ")" |
                       (<class-name> | <var-name>) "." <subroutine-name> "(" <expression-list> ")"
<expression-list>  ::= {<expression {"," <expression>}*}?
<op>               ::= "+" | "-" | "*" | "/" | "&" | "|" | "<" | ">" | "="
<unary-op>         ::= "-" | "~"
<keyword-constant> ::= "true" | "false" | "null" | "this"

clang-format on
*/

typedef enum class ParseTreeNodeType_s {
  P_UNDEFINED,
  P_ARRAY_VAR,
  P_CLASS_OR_VAR_NAME,
  P_DELIMITER,
  P_EXPRESSION,
  P_EXPRESSION_LIST,
  P_INTEGER_CONSTANT,
  P_KEYWORD,
  P_KEYWORD_CONSTANT,
  P_LEFT_BRACKET,
  P_LEFT_PARENTHESIS,
  P_OP,
  P_RETURN_STATEMENT,
  P_RIGHT_BRACKET,
  P_RIGHT_PARENTHESIS,
  P_SCALAR_VAR,
  P_STATEMENT_LIST,
  P_STRING_CONSTANT,
  P_SUBROUTINE_CALL,
  P_SUBROUTINE_NAME,
  P_TERM,
  P_UNARY_OP,
} ParseTreeNodeType_t;

class ParseTreeNode : public std::enable_shared_from_this<ParseTreeNode> {
  // class ParseTreeNode {
public:
  ParseTreeNode() = delete;
  ParseTreeNode(const ParseTreeNode&) = delete;
  ParseTreeNode& operator=(const ParseTreeNode&) = delete;

  virtual ~ParseTreeNode() = default;

  const ParseTreeNodeType_t type{ParseTreeNodeType_t::P_UNDEFINED};

  static std::string to_string(ParseTreeNodeType_t);

  friend std::ostream& operator<<(std::ostream&, const ParseTreeNode&);

protected:
  ParseTreeNode(ParseTreeNodeType_t type) : type(type) {}

private:
  std::shared_ptr<ParseTreeNode> parent{nullptr};
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

  ParseTree(ParseTreeNodeType_t type,
            std::unique_ptr<std::vector<JackToken>>& tokens)
      : root(std::make_shared<ParseTreeNonTerminal>(type)), tokens(tokens)
  {
  }

  std::shared_ptr<ParseTreeNonTerminal> parse_statement();
  std::shared_ptr<ParseTreeNonTerminal> parse_statements();
  std::shared_ptr<ParseTreeNonTerminal> parse_return_statement();

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
};
