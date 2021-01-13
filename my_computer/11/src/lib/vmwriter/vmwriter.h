#pragma once

#include "parser/parse_tree.h"

class VmWriter {
public:
  VmWriter() = delete;
  VmWriter(const VmWriter&) = delete;
  VmWriter& operator=(const VmWriter&) = delete;

  virtual ~VmWriter() = default;

  // friend std::ostream& operator<<(std::ostream&, const VmWriter&);

  // Return visited node of parse tree using DFS
  const ParseTreeNode* visit();

  void init()
  {
    unvisited_nodes.clear();
    unvisited_nodes.push_back(parsetree_root_node);
  }

  VmWriter(const std::shared_ptr<ParseTreeNonTerminal> pt)
      : parsetree_root_node(&(*pt))
  {
    init();
  }

private:
  const ParseTreeNode* parsetree_root_node;
  std::vector<const ParseTreeNode*> unvisited_nodes;
};



/*
AST Structure

Program Structure:

<class-decl>       ::= <class-name> {<class-var-decl>}* {<subroutine-decl>}*
<class-var-decl>   ::= <storage-class> <variable-list>
<storage-class>    ::= "static" | "field"
<subroutine-decl>  ::= <subroutine-type> <return-type> <subroutine-name>
                       <parameter-list> <subroutine-body>
<subroutine-type>  ::= "constructor" | "function" | "method"
<return-type>      ::= "void" | <type>
<parameter-list>   ::= {<variable>}*
<subroutine-body>  ::= {<var-decl-block>}? <statement-block>
<var-decl-block>   ::= <variable> {<variable>}*
<variable-list>    ::= <variable> {<variable>}*
<variable>         ::= <type> <var-name>
<type>             ::= "int" | "char" | "boolean" | <class-name>
<class-name>       ::= <identifier>
<subroutine-name>  ::= <identifier>
<var-name>         ::= <identifier>

Statements:

<statement-block>  ::= {<statement>}*
<statement>        ::= <let-statement> | <if-statement> | <while-statement> |
                       <do-statement> | <return-statement>
<let-statement>    ::= (<scalar-var> | <array-var>) <expression>
<if-statement>     ::= <expression <statement-block> {<statement-block>}?
<while-statement>  ::= <expression> <statement-block>
<do-statement>     ::= <subroutine-call>
<return-statement> ::= <expression>

Expressions:

<expression>       ::= <term> {<op> <term>}*
<term>             ::= <integer-constant> | <string-constant> | <keyword-constant> |
                       <scalar-var> | <array-var> |
                       <subroutine-call> | <expression> | <unary-op> <term>
<scalar-var>       ::= <var-name>
<array-var>        ::= <var-name> <expression>
<index-expression> ::= <expression>
<subroutine-call>  ::= <internal-subroutine-call> | <external-subroutine-call>
<internal-subroutine-call> ::= <subroutine-name> <expression-list>
<external-subroutine-call> ::= <subroutine-owner> <subroutine-name> <expression-list>
<subroutine-owner> ::= <class-name> | <var-name>
<expression-list>  ::= {<expression>}*
<op>               ::= "+" | "-" | "*" | "/" | "&" | "|" | "<" | ">" | "="
<unary-op>         ::= "-" | "~"
<keyword-constant> ::= "true" | "false" | "null" | "this"

symbol table CLASS
------------------
static/field
name
type
id

symbol table SUBROUTINE
------------------
argument/local
name
type
id
*/
