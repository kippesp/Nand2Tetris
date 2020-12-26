#include <string>

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
<subroutine-body>  ::= "{" {<var-decl>}* <statements "}"
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
<expression-list>  ::= {<expression
                       {"," <expression>}*}?
<op>               ::= "+" | "-" | "*" | "/" | "&" | "|" | "<" | ">" | "="
<unary-op>         ::= "-" | "~"
<keyword-constant> ::= "true" | "false" | "null" | "this"

clang-format on
*/

#if 0
typedef enum class TreeNodeType_s {
  N_CLASS,
} TreeNodeType_t;
#endif

class Node {
public:
  Node() = delete;
  Node(const Node&) = delete;
  Node& operator=(const Node&) = delete;

  const std::string name;

private:
  int nesting_level{0};

  // children_nodes
};

#include "tokenizer/jack_tokenizer.h"

class Class : public Node {
public:
  Class() = delete;
  Class(const Class&) = delete;
  Class& operator=(const Class&) = delete;

  Class(std::unique_ptr<std::vector<JackToken>>);
};
