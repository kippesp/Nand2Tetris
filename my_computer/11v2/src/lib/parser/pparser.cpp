#include "pparser.h"

#include <cassert>
#include <list>
#include <optional>

#include <signal.h>

using namespace ast;

namespace pratt {

AstNodeRef Parser::create_ast_node(AstNodeType_t type)
{
  return AST.add(AstNode(type));
}

#if 0
static ast::AstNodeRef NMPrefixP1()
{
  //
}

static ast::AstNodeRef NMInfixP1(ast::AstNodeRef left_operand_tree)
{
  //
}
#endif

ast::AstNodeRef Parser::prefixParser()
{
  //

  return EmptyNodeRef;
}

ast::AstNodeRef Parser::infixParser(ast::AstNodeRef left_parse_tree)
{
  //

  (void)left_parse_tree;

  return EmptyNodeRef;
}

// prefix parser
ast::AstNodeRef Parser::integer_handler()
{
  assert(current_token.get().value_enum == TokenValue_t::J_INTEGER_CONSTANT);

  AstNodeRef AstBranch = create_ast_node(AstNodeType_t::N_INTEGER_CONSTANT,
                                         current_token.get().value_str);

  get_next_token();

  return AstBranch;
}

// <expression> ::= <term> {<binary-op> <term>}*
// <term> ::= <integer-constant> | <string-constant> | <keyword-constant> |
//            <scalar-var> | <array-var> | <subroutine-call> |
//            "(" <expression> ")" | <unary-op> <term>
ast::AstNodeRef Parser::parse_expression(precedence_t precedence = 0)
{
  // Get current token
  // If token has an infix parser, call it

  (void)precedence;

  // if (current_token.get().value_enum == TokenValue_t::J_IDENTIFIER)
  // if (current_token.get().value_enum == TokenValue_t::J_INTEGER_CONSTANT)
  //{
  //}

  return EmptyNodeRef;
}

Parser::Parser(Tokens_t& tokens)
    : token_iter(tokens.begin()),
      token_iter_end(tokens.end()),
      current_token(*token_iter),
      peek_token(*(++token_iter))
{
#if 0
  ParserMap[TokenValue_t::J_INTEGER_CONSTANT] =
      TokenParsers_t(std::bind(&Parser::integerHandler, this), nullptr, 1);
#endif

  // PrattParserMap_t PrattParserMap;

  // auto V = TokenParsers_t(this->Test1, this->Test1, 1);
  //   auto fp = std::bind(&aClass::test, a, _1, _2);

  // ParserFn_t* F1 = &Parser::Test1;

  // ast::AstNodeRef N = AST.add(AstNode(AstNodeType_t::N_CLASS_DECL));

  // const PrefixParserFn_t F1 = &Parser::PrefixP1;

#if 0
  using ParserPrefixMap_t = std::map<TokenValue_t, PrefixParserFn_t>;
  // using ParserInfixMap_t = std::map<TokenValue_t, InfixParserFn_t>;

  ParserPrefixMap_t ParserPrefixMap;

  ParserPrefixMap.insert({TokenValue_t::J_PLUS, NMPrefixP1});

  using PrefixParserFn2_t = std::function<ast::AstNodeRef(Parser*)>;
  using ParserPrefixMap2_t = std::map<TokenValue_t, PrefixParserFn2_t>;

  ParserPrefixMap2_t ParserPrefixMap2;

  PrefixParserFn2_t Fn2 = std::bind(&Parser::PrefixP1, this);

  ParserPrefixMap2.insert({TokenValue_t::J_PLUS, Fn2});
#endif

  // ParserFn_t PF1 = nullptr;

  // auto FP1 = std::bind(&Parser::Test1, this, N, 1);
  // auto FP2 = std::bind(&Parser::Test1, this, N, 0);

  //  auto PF1 = ParserFn_t(FP1, 1);

  // ParserFn_t F = ParserFn_t(FP1);

  //  auto TP = TokenParsers_t(FP1, FP2, 1);

  // PrattParserMap.insert(TokenParsers_t(FP1, FP2, 1));
}

}  // namespace pratt
