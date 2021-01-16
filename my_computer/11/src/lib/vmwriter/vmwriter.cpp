#include "vmwriter/vmwriter.h"

#include <algorithm>
#include <cassert>

#include "parser/parse_tree.h"
#include "semantic_exception.h"

using namespace std;

// bool: 0xFFFF


const ParseTreeNode* VmWriter::visit()
{
  if (unvisited_nodes.size() == 0) return nullptr;

  auto node = unvisited_nodes.back();
  unvisited_nodes.pop_back();

  if (auto term_node = dynamic_cast<const ParseTreeNonTerminal*>(node))
  {
    std::vector<std::shared_ptr<ParseTreeNode>> ncopy;
    for (auto child_node : term_node->get_child_nodes())
      ncopy.push_back(child_node);
    reverse(ncopy.begin(), ncopy.end());

    for (auto child_node : ncopy) unvisited_nodes.push_back(&(*child_node));
  }

  return node;
}

// TODO:
// https://stackoverflow.com/questions/15911890/overriding-return-type-in-function-template-specialization
const ParseTreeTerminal* VmWriter::find_first_term_node(
    ParseTreeNodeType_t type, const ParseTreeNonTerminal* nt_node) const
{
  auto child_nodes = nt_node->get_child_nodes();
  for (auto node : child_nodes)
  {
    if ((*node).type == type)
    {
      auto rvalue = dynamic_cast<const ParseTreeTerminal*>(&(*node));
      assert(rvalue && "Not a terminal node");

      return rvalue;
    }
  }

  return nullptr;
}

const ParseTreeNonTerminal* VmWriter::find_first_nonterm_node(
    ParseTreeNodeType_t type, const ParseTreeNonTerminal* nt_node) const
{
  auto child_nodes = nt_node->get_child_nodes();
  for (auto node : child_nodes)
  {
    if ((*node).type == type)
    {
      auto rvalue = dynamic_cast<const ParseTreeNonTerminal*>(&(*node));
      assert(rvalue && "Not a non-terminal node");

      return rvalue;
    }
  }

  return nullptr;
}

const string VmWriter::get_class_name(const ParseTreeNonTerminal* nt_node) const
{
  assert(nt_node->type == ParseTreeNodeType_t::P_CLASS_DECL_BLOCK);
  auto class_name_node =
      find_first_term_node(ParseTreeNodeType_t::P_CLASS_NAME, nt_node);
  const string name = class_name_node->token.value_str;

  return name;
}

void VmWriter::create_classvar_symtable(const ParseTreeNonTerminal* nt_node)
{
  assert(nt_node->type == ParseTreeNodeType_t::P_CLASS_DECL_BLOCK);
  auto classvar_decl_block = find_first_nonterm_node(
      ParseTreeNodeType_t::P_CLASSVAR_DECL_BLOCK, nt_node);
  for (auto node : classvar_decl_block->get_child_nodes())
  {
    if ((*node).type == ParseTreeNodeType_t::P_CLASSVAR_DECL_STATEMENT)
    {
      auto classvar_decl_statement =
          dynamic_cast<const ParseTreeNonTerminal*>(&(*node));
      assert(classvar_decl_statement && "Not a non-terminal node");

      auto classvar_scope_node = find_first_term_node(
          ParseTreeNodeType_t::P_CLASSVAR_SCOPE, classvar_decl_statement);
      auto vardecl_node = find_first_nonterm_node(
          ParseTreeNodeType_t::P_VARIABLE_DECL, classvar_decl_statement);
      auto vartype_node = find_first_term_node(
          ParseTreeNodeType_t::P_VARIABLE_TYPE, vardecl_node);

      Symbol::StorageClass_t storage_class;

      if (classvar_scope_node->token.value_enum == TokenValue_t::J_STATIC)
        storage_class = Symbol::ClassStorageClass_t::S_STATIC;
      else if (classvar_scope_node->token.value_enum == TokenValue_t::J_FIELD)
        storage_class = Symbol::ClassStorageClass_t::S_FIELD;
      else
        assert(0 && "Invalid storage class in token");

      Symbol::VariableType_t variable_type =
          Symbol::variable_type_from_token(vartype_node->token);

      if (const auto pBT = get_if<Symbol::BasicType_t>(&variable_type))
        if (*pBT == Symbol::BasicType_t::T_VOID)
          throw SemanticException("void type not allowed here");

      auto variable_list = find_first_nonterm_node(
          ParseTreeNodeType_t::P_VARIABLE_LIST, vardecl_node);

      for (auto variable_list_cnodes : variable_list->get_child_nodes())
      {
        if ((*variable_list_cnodes).type ==
            ParseTreeNodeType_t::P_VARIABLE_NAME)
        {
          auto varname_node =
              dynamic_cast<const ParseTreeTerminal*>(&(*variable_list_cnodes));

          string varname = varname_node->token.value_str;

          symbol_table.add_symbol(Symbol::ScopeLevel_t::CLASS, varname,
                                  variable_type, storage_class);
        }
      }
    }
  }
}

void VmWriter::create_subroutine_symtable(const ParseTreeNonTerminal* sub_node)
{
  assert(sub_node->type == ParseTreeNodeType_t::P_SUBROUTINE_DECL_BLOCK);

  if (auto sub_type_node = find_first_term_node(
          ParseTreeNodeType_t::P_SUBROUTINE_TYPE, sub_node);
      sub_type_node->token.value_enum == TokenValue_t::J_METHOD)
  {
    symbol_table.add_symbol(Symbol::ScopeLevel_t::SUBROUTINE, "this",
                            Symbol::ClassType_t{class_name},
                            Symbol::SubroutineStorageClass_t::S_ARGUMENT);
  }

  // Add any argument variables to the symbol table
  {
    auto parmlist_node = find_first_nonterm_node(
        ParseTreeNodeType_t::P_PARAMETER_LIST, sub_node);
    auto nodes = parmlist_node->get_child_nodes();

    for (decltype(nodes)::iterator it = nodes.begin(); it != nodes.end(); ++it)
    {
      // All child nodes are terminals
      auto pTermNode = dynamic_cast<const ParseTreeTerminal*>(&(*(*it)));
      assert(pTermNode);

      if (pTermNode->type == ParseTreeNodeType_t::P_VARIABLE_TYPE)
      {
        const decltype(pTermNode) vartype_node = pTermNode;
        Symbol::VariableType_t variable_type =
            Symbol::variable_type_from_token(vartype_node->token);

        if (const auto pBT = get_if<Symbol::BasicType_t>(&variable_type))
          if (*pBT == Symbol::BasicType_t::T_VOID)
            throw SemanticException("void type not allowed here");

        ++it;
        auto pVarnameNode = dynamic_cast<const ParseTreeTerminal*>(&(*(*it)));
        assert(pVarnameNode);
        assert(pVarnameNode->type == ParseTreeNodeType_t::P_VARIABLE_NAME);

        string varname = pVarnameNode->token.value_str;

        symbol_table.add_symbol(Symbol::ScopeLevel_t::SUBROUTINE, varname,
                                variable_type,
                                Symbol::SubroutineStorageClass_t::S_ARGUMENT);
      }
    }
  }

  // Add any subroutine local variables to the symbol table
  {
    auto sub_body = find_first_nonterm_node(
        ParseTreeNodeType_t::P_SUBROUTINE_BODY, sub_node);
    auto var_decl_block = find_first_nonterm_node(
        ParseTreeNodeType_t::P_VAR_DECL_BLOCK, sub_body);
    auto var_decl_block_nodes = var_decl_block->get_child_nodes();

    // Process all P_VAR_DECL_BLOCK children nodes
    for (decltype(var_decl_block_nodes)::iterator it =
             var_decl_block_nodes.begin();
         it != var_decl_block_nodes.end(); ++it)
    {
      auto pVarDeclStatement =
          dynamic_cast<const ParseTreeNonTerminal*>(&(*(*it)));
      if (!pVarDeclStatement) continue;
      assert(pVarDeclStatement->type ==
             ParseTreeNodeType_t::P_VAR_DECL_STATEMENT);

      auto pVarDecl = find_first_nonterm_node(
          ParseTreeNodeType_t::P_VARIABLE_DECL, pVarDeclStatement);
      auto pVarType =
          find_first_term_node(ParseTreeNodeType_t::P_VARIABLE_TYPE, pVarDecl);
      assert(pVarType);

      Symbol::VariableType_t variable_type =
          Symbol::variable_type_from_token(pVarType->token);

      if (const auto pBT = get_if<Symbol::BasicType_t>(&variable_type))
        if (*pBT == Symbol::BasicType_t::T_VOID)
          throw SemanticException("void type not allowed here");

      auto pVarList = find_first_nonterm_node(
          ParseTreeNodeType_t::P_VARIABLE_LIST, pVarDecl);
      auto var_list_children_nodes = pVarList->get_child_nodes();

      // Process all P_VARIABLE_LIST children nodes
      for (decltype(var_list_children_nodes)::iterator it2 =
               var_list_children_nodes.begin();
           it2 != var_list_children_nodes.end(); ++it2)
      {
        // All child nodes are terminals
        auto pTermNode = dynamic_cast<const ParseTreeTerminal*>(&(*(*it2)));
        assert(pTermNode);

        if (pTermNode->type == ParseTreeNodeType_t::P_VARIABLE_NAME)
        {
          string varname = pTermNode->token.value_str;
          symbol_table.add_symbol(Symbol::ScopeLevel_t::SUBROUTINE, varname,
                                  variable_type,
                                  Symbol::SubroutineStorageClass_t::S_LOCAL);
        }
      }
    }
  }
}

SubroutineDescr::SubroutineDescr(const VmWriter& VM,
                                 const ParseTreeNonTerminal* pSubDecl)
{
  auto sub_type_node =
      VM.find_first_term_node(ParseTreeNodeType_t::P_SUBROUTINE_TYPE, pSubDecl);
  auto sub_return_type_node =
      VM.find_first_term_node(ParseTreeNodeType_t::P_RETURN_TYPE, pSubDecl);
  auto sub_name_node =
      VM.find_first_term_node(ParseTreeNodeType_t::P_SUBROUTINE_NAME, pSubDecl);
  pBody = VM.find_first_nonterm_node(ParseTreeNodeType_t::P_SUBROUTINE_BODY,
                                     pSubDecl);

  type = sub_type_node->token.value_enum;
  return_type = Symbol::variable_type_from_token(sub_return_type_node->token);
  name = sub_name_node->token.value_str;
}

void VmWriter::lower_class()
{
  symbol_table.reset(Symbol::ScopeLevel_t::CLASS);
  symbol_table.reset(Symbol::ScopeLevel_t::SUBROUTINE);

  while (auto node = visit())
  {
    if (node->type == ParseTreeNodeType_t::P_CLASS_DECL_BLOCK)
    {
      auto nonterm_node = dynamic_cast<const ParseTreeNonTerminal*>(node);
      class_name = get_class_name(nonterm_node);
      create_classvar_symtable(nonterm_node);
    }
    else if (node->type == ParseTreeNodeType_t::P_SUBROUTINE_DECL_BLOCK)
    {
      auto nonterm_node = dynamic_cast<const ParseTreeNonTerminal*>(node);
      symbol_table.reset(Symbol::ScopeLevel_t::SUBROUTINE);
      lower_subroutine(nonterm_node);
    }
  }
}

void VmWriter::lower_subroutine(const ParseTreeNonTerminal* pSubDeclBlockNode)
{
  SubroutineDescr sub_descr = SubroutineDescr(*this, pSubDeclBlockNode);

  create_subroutine_symtable(pSubDeclBlockNode);

  auto pStatementListNode = find_first_nonterm_node(
      ParseTreeNodeType_t::P_STATEMENT_LIST, sub_descr.pBody);

  auto childNodes = pStatementListNode->get_child_nodes();

  if (childNodes.size() == 0) throw SemanticException("empty subroutine body");

  for (auto child : childNodes)
  {
    auto pS = dynamic_cast<const ParseTreeNonTerminal*>(&(*child));

    switch ((*child).type)
    {
      case ParseTreeNodeType_t::P_DO_STATEMENT:
        break;
      case ParseTreeNodeType_t::P_IF_STATEMENT:
        throw SemanticException("unknown statement");
      case ParseTreeNodeType_t::P_LET_STATEMENT:
        throw SemanticException("unknown statement");
      case ParseTreeNodeType_t::P_RETURN_STATEMENT:
        // TODO
        break;
      case ParseTreeNodeType_t::P_WHILE_STATEMENT:
        throw SemanticException("unknown statement");
      default:
        assert(0 && "fallthrough");
        throw SemanticException("Unexpected node type");
    }
  }

#if 0
  TokenValue_t subroutine_type = TokenValue_t::J_UNDEFINED;

  while (auto node = visit())
  {
    if (node->type == ParseTreeNodeType_t::P_SUBROUTINE_TYPE)
    {
      auto subroutine_type_node =
      find_first_term_node(ParseTreeNodeType_t::P_CLASS_NAME, nt_node);
       
        node->token.value_enum;


      subroutine_type = node->token.value_enum;
  const string name = class_name_node->token.value_str;


  auto class_name_node =
      find_first_term_node(ParseTreeNodeType_t::P_CLASS_NAME, nt_node);
  const string name = class_name_node->token.value_str;

const ParseTreeTerminal* VmWriter::find_first_term_node(
    ParseTreeNodeType_t type, const ParseTreeNonTerminal* nt_node) const

    }
  }
#endif
}

#if 0
(P_CLASS_DECL_BLOCK 
 (P_KEYWORD class)
 (P_CLASS_NAME PongGame)
 (P_DELIMITER <left_brace>)
 (P_CLASSVAR_DECL_BLOCK 
  (P_CLASSVAR_DECL_STATEMENT 
   (P_CLASSVAR_SCOPE static)
   (P_VARIABLE_DECL 
    (P_VARIABLE_TYPE PongGame)
    (P_VARIABLE_LIST 
     (P_VARIABLE_NAME instance)))
   (P_DELIMITER <semicolon>))
  (P_CLASSVAR_DECL_STATEMENT 
   (P_CLASSVAR_SCOPE field)
   (P_VARIABLE_DECL 
    (P_VARIABLE_TYPE Bat)
    (P_VARIABLE_LIST 
     (P_VARIABLE_NAME bat)))
   (P_DELIMITER <semicolon>))
  (P_CLASSVAR_DECL_STATEMENT 
   (P_CLASSVAR_SCOPE field)
   (P_VARIABLE_DECL 
    (P_VARIABLE_TYPE integer)
    (P_VARIABLE_LIST 
     (P_VARIABLE_NAME wall1)
     (P_DELIMITER <comma>)
     (P_VARIABLE_NAME wall2)))
   (P_DELIMITER <semicolon>))
  (P_CLASSVAR_DECL_STATEMENT 
   (P_CLASSVAR_SCOPE field)
   (P_VARIABLE_DECL 
    (P_VARIABLE_TYPE boolean)
    (P_VARIABLE_LIST 
     (P_VARIABLE_NAME exit)))
   (P_DELIMITER <semicolon>)))
 (P_SUBROUTINE_DECL_BLOCK 
  (P_SUBROUTINE_TYPE method)
  (P_RETURN_TYPE void)
  (P_SUBROUTINE_NAME moveBall)
  (P_DELIMITER <left_parenthesis>)
  (P_PARAMETER_LIST )
  (P_DELIMITER <right_parenthesis>)
  (P_SUBROUTINE_BODY 
   (P_DELIMITER <left_brace>)
   (P_VAR_DECL_BLOCK 
    (P_VAR_DECL_STATEMENT 
     (P_KEYWORD var)
     (P_VARIABLE_DECL 
      (P_VARIABLE_TYPE integer)
      (P_VARIABLE_LIST 
       (P_VARIABLE_NAME bouncingDirection)
       (P_DELIMITER <comma>)
       (P_VARIABLE_NAME batLeft)))
     (P_DELIMITER <semicolon>))
    (P_VAR_DECL_STATEMENT 
     (P_KEYWORD var)
     (P_VARIABLE_DECL 
      (P_VARIABLE_TYPE boolean)
      (P_VARIABLE_LIST 
       (P_VARIABLE_NAME ball)))
     (P_DELIMITER <semicolon>)))
   (P_STATEMENT_LIST 
    (P_LET_STATEMENT 
     (P_KEYWORD let)
     (P_SCALAR_VAR bouncingDirection)
     (P_OP <equal>)
     (P_EXPRESSION 
      (P_TERM 
       (P_INTEGER_CONSTANT 1)))
     (P_DELIMITER <semicolon>))
    (P_RETURN_STATEMENT 
     (P_KEYWORD return)
     (P_DELIMITER <semicolon>)))
   (P_DELIMITER <right_brace>)))
 (P_DELIMITER <right_brace>))
#endif

#if 0
class Main {
   function void main() {
      do Output.printInt(1 + (2 * 3));
      return;
   }
}

(P_CLASS_DECL_BLOCK 
 (P_KEYWORD class)
 (P_CLASS_NAME Main)
 (P_DELIMITER <left_brace>)

 (P_SUBROUTINE_DECL_BLOCK 
  (P_SUBROUTINE_TYPE function)
  (P_RETURN_TYPE void)
  (P_SUBROUTINE_NAME main)
  (P_DELIMITER <left_parenthesis>)
  (P_PARAMETER_LIST )
  (P_DELIMITER <right_parenthesis>)
  (P_SUBROUTINE_BODY 
   (P_DELIMITER <left_brace>)

   (P_STATEMENT_LIST 
    (P_DO_STATEMENT 
     (P_KEYWORD do)
     (P_SUBROUTINE_CALL 
      (P_CLASS_OR_VAR_NAME Output)
      (P_DELIMITER <period>)
      (P_SUBROUTINE_NAME printInt)
      (P_DELIMITER <left_parenthesis>)
      (P_EXPRESSION_LIST 
       (P_EXPRESSION 
        (P_TERM 
         (P_INTEGER_CONSTANT 1))
        (P_OP <plus>)
        (P_TERM 
         (P_DELIMITER <left_parenthesis>)
         (P_EXPRESSION 
          (P_TERM 
           (P_INTEGER_CONSTANT 2))
          (P_OP <asterisk>)
          (P_TERM 
           (P_INTEGER_CONSTANT 3)))
         (P_DELIMITER <right_parenthesis>))))
      (P_DELIMITER <right_parenthesis>))
     (P_DELIMITER <semicolon>))
    (P_RETURN_STATEMENT 
     (P_KEYWORD return)
     (P_DELIMITER <semicolon>)))

   (P_DELIMITER <right_brace>)))
 (P_DELIMITER <right_brace>))
#endif
