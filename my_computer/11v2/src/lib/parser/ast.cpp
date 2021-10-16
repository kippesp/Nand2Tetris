#include "ast.h"

#include <cassert>

//#include "parse_tree.h"

namespace ast {

std::ostream& operator<<(std::ostream& os, const AstNode& rhs)
{
  os << "type: ";

  os << AstNode::to_string(rhs.type) << std::endl;

  if (const auto s_ptr(std::get_if<std::string>(&rhs.value)); s_ptr)
  {
    os << "value (string): ";
    os << *s_ptr << std::endl;
  }
  else if (const auto i_ptr(std::get_if<int>(&rhs.value)); i_ptr)
  {
    os << "value (int): ";
    os << *i_ptr << std::endl;
  }

  // os << "token_value_str: ";
  // os << "<<< " << rhs.value_str << " >>>" << std::endl;

  return os;
}

std::string AstNode::to_string(AstNodeType_t t)
{
  switch (t)
  {
    // clang-format off
    case AstNodeType_t::N_UNDEFINED:                   return "UNDEFINED";

    case AstNodeType_t::N_CLASS_DECL:                  return "CLASS_DECL";
    case AstNodeType_t::N_EXPRESSION:                  return "EXPRESSION";
    case AstNodeType_t::N_INTEGER_CONSTANT:            return "INTEGER_CONSTANT";
    case AstNodeType_t::N_RETURN_STATEMENT:            return "RETURN_STATEMENT";
    case AstNodeType_t::N_STATEMENT_BLOCK:             return "STATEMENT_BLOCK";

    case AstNodeType_t::N_FUNCTION_DECL:               return "FUNCTION_DECL";
    case AstNodeType_t::N_METHOD_DECL:                 return "METHOD_DECL";
    case AstNodeType_t::N_CONSTRUCTOR_DECL:            return "CONSTRUCTOR_DECL";

    case AstNodeType_t::N_SUBROUTINE_BODY:             return "SUBROUTINE_BODY";
    case AstNodeType_t::N_SUBROUTINE_DESCR:            return "SUBROUTINE_DESCR";
    case AstNodeType_t::N_RETURN_TYPE:                 return "RETURN_TYPE";
    case AstNodeType_t::N_INPUT_PARAMETERS:            return "INPUT_PARAMETERS";

    case AstNodeType_t::N_KEYWORD_CONSTANT:            return "KEYWORD_CONSTANT";
    case AstNodeType_t::N_STRING_CONSTANT:             return "STRING_CONSTANT";

    case AstNodeType_t::N_TRUE_KEYWORD:                return "TRUE_KEYWORD";
    case AstNodeType_t::N_FALSE_KEYWORD:               return "FALSE_KEYWORD";
    case AstNodeType_t::N_NULL_KEYWORD:                return "NULL_KEYWORD";
    case AstNodeType_t::N_THIS_KEYWORD:                return "THIS_KEYWORD";
    case AstNodeType_t::N_LET_STATEMENT:               return "LET_STATEMENT";
    case AstNodeType_t::N_DO_STATEMENT:                return "DO_STATEMENT";
    case AstNodeType_t::N_WHILE_STATEMENT:             return "WHILE_STATEMENT";
    case AstNodeType_t::N_IF_STATEMENT:                return "IF_STATEMENT";

    case AstNodeType_t::N_CLASS_VARIABLES:             return "CLASS_VARIABLES";
    case AstNodeType_t::N_LOCAL_VARIABLES:             return "LOCAL_VARIABLES";
    case AstNodeType_t::N_VARIABLE_TYPE:               return "VARIABLE_TYPE";
    case AstNodeType_t::N_VARIABLE_DECL:               return "VARIABLE_DECL";

    case AstNodeType_t::N_SUBSCRIPTED_VARIABLE_NAME:   return "SUBSCRIPTED_VARIABLE_NAME";
    case AstNodeType_t::N_VARIABLE_NAME:               return "VARIABLE_NAME";

    case AstNodeType_t::N_OP_MULTIPLY:                 return "OP_MULTIPLY";
    case AstNodeType_t::N_OP_DIVIDE:                   return "OP_DIVIDE";
    case AstNodeType_t::N_OP_ADD:                      return "OP_ADD";
    case AstNodeType_t::N_OP_SUBTRACT:                 return "OP_SUBTRACT";
    case AstNodeType_t::N_OP_LOGICAL_EQUALS:           return "OP_LOGICAL_EQUALS";
    case AstNodeType_t::N_OP_LOGICAL_GT:               return "OP_LOGICAL_GT";
    case AstNodeType_t::N_OP_LOGICAL_LT:               return "OP_LOGICAL_LT";
    case AstNodeType_t::N_OP_LOGICAL_AND:              return "OP_LOGICAL_AND";
    case AstNodeType_t::N_OP_LOGICAL_OR:               return "OP_LOGICAL_OR";

    case AstNodeType_t::N_OP_PREFIX_NEG:               return "OP_PREFIX_NEG";
    case AstNodeType_t::N_OP_PREFIX_LOGICAL_NOT:       return "OP_PREFIX_LOGICAL_NOT";

    case AstNodeType_t::N_SUBROUTINE_CALL:             return "SUBROUTINE_CALL";
    case AstNodeType_t::N_SUBROUTINE_NAME:             return "SUBROUTINE_NAME";
    case AstNodeType_t::N_LOCAL_CALL_SITE:             return "LOCAL_CALL_SITE";
    case AstNodeType_t::N_GLOBAL_CALL_SITE:            return "GLOBAL_CALL_SITE";
    case AstNodeType_t::N_GLOBAL_BIND_NAME:            return "GLOBAL_BIND_NAME";

    case AstNodeType_t::N_CLASS_VARIABLE_SCOPE:        return "CLASS_VARIABLE_SCOPE";
      // clang-format on
  }

  assert(0 && "fallthrough");
}

bool AstNode::operator==(const AstNode& other) const
{
  return this == &other;
}

std::string AstNode::as_s_expression(const std::string& indent)
{
  std::stringstream ss;

  ss << indent << "(" << to_string(type);

  if (const auto s_ptr(std::get_if<std::string>(&value)); s_ptr)
  {
    ss << " string_value:" << *s_ptr;
  }
  else if (const auto i_ptr(std::get_if<int>(&value)); i_ptr)
  {
    ss << " int_value:" << *i_ptr;
  }

  for (const auto& N : child_nodes)
  {
    ss << "\n" << N.get().as_s_expression(indent + "  ");
  }

  ss << ")";

  return ss.str();
}

void AstNode::dump()
{
  auto const& output = as_s_expression();

  std::cout << output << std::endl;
}

AstNode::AstNode(AstNodeType_t node_type) : type(node_type) {}

std::vector<AstNodeCRef> AstNode::get_child_nodes() const
{
  std::vector<AstNodeCRef> V;

  for (const auto& N : child_nodes)
  {
    V.emplace_back(std::cref(N.get()));
  }

  return V;
}

AstNodeRef AstNode::add_child(AstNodeRef node)
{
  AstNodeRef r = child_nodes.emplace_back(node);
  return r;
}

AstNodeRef AstTree::add(const AstNode& node)
{
  /*
   * TODO: Check this on Windows for warning/notice
  auto& added_node =
      nodes.emplace_back(std::move(std::make_unique<AstNode>(node)));
  */
  auto& added_node = nodes.emplace_back(std::make_unique<AstNode>(node));
  return *added_node;
}

}  // namespace ast
