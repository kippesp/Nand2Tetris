#include "vmwriter/vmwriter.h"

#include <signal.h>

#include <algorithm>
#include <cassert>
#include <queue>

#include "parser/parse_tree.h"
#include "semantic_exception.h"

using namespace std;

// seven
// dec-to-bin
// square dance
// average
// pong
// complex arrays

static string get_term_node_str(const ParseTreeNode* pN)
{
  auto pTN = dynamic_cast<const ParseTreeTerminal*>(pN);
  assert(pTN && "Non-term node");
  return pTN->token.value_str;
}

const ParseTreeNode* VmWriter::visit()
{
  if (unvisited_nodes.size() == 0) return nullptr;

  auto node = unvisited_nodes.back();
  unvisited_nodes.pop_back();

  if (auto nonterm_node = dynamic_cast<const ParseTreeNonTerminal*>(node))
  {
    std::vector<std::shared_ptr<ParseTreeNode>> ncopy;
    for (auto child_node : nonterm_node->get_child_nodes())
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
  const string name = get_term_node_str(class_name_node);

  return name;
}

void VmWriter::create_classvar_symtable(const ParseTreeNonTerminal* nt_node)
{
  assert(nt_node->type == ParseTreeNodeType_t::P_CLASS_DECL_BLOCK);
  auto pClassVarDeclBlock = find_first_nonterm_node(
      ParseTreeNodeType_t::P_CLASSVAR_DECL_BLOCK, nt_node);

  if (!pClassVarDeclBlock) return;

  for (auto node : pClassVarDeclBlock->get_child_nodes())
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
          const string varname = get_term_node_str(&(*variable_list_cnodes));

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
    auto pSubroutineBody = find_first_nonterm_node(
        ParseTreeNodeType_t::P_SUBROUTINE_BODY, sub_node);
    auto pVarDeclBlock = find_first_nonterm_node(
        ParseTreeNodeType_t::P_VAR_DECL_BLOCK, pSubroutineBody);

    if (!pVarDeclBlock) return;

    auto pVarDeclBlockNodes = pVarDeclBlock->get_child_nodes();

    // Process all P_VAR_DECL_BLOCK children nodes
    for (decltype(pVarDeclBlockNodes)::iterator it = pVarDeclBlockNodes.begin();
         it != pVarDeclBlockNodes.end(); ++it)
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

  structured_control_id = 0;
}

void VmWriter::lower_class()
{
  symbol_table.reset(Symbol::ScopeLevel_t::CLASS);
  symbol_table.reset(Symbol::ScopeLevel_t::SUBROUTINE);

  class_name = get_class_name(pClassRootNode);
  create_classvar_symtable(pClassRootNode);

  auto child_nodes = pClassRootNode->get_child_nodes();
  for (auto node : child_nodes)
  {
    if ((*node).type == ParseTreeNodeType_t::P_SUBROUTINE_DECL_BLOCK)
    {
      auto pN = dynamic_cast<const ParseTreeNonTerminal*>(&(*node));
      lower_subroutine(pN);
    }
  }
}

void VmWriter::lower_return_statement(const ParseTreeNonTerminal* pStatement)
{
  auto pExpression =
      find_first_nonterm_node(ParseTreeNodeType_t::P_EXPRESSION, pStatement);

  if ((pSubDescr->type == TokenValue_t::J_FUNCTION) &&
      get_if<Symbol::BasicType_t>(&pSubDescr->return_type) &&
      (get<Symbol::BasicType_t>(pSubDescr->return_type) ==
       Symbol::BasicType_t::T_VOID))
  {
    if (pExpression)
      throw SemanticException("Function declared void but returning non-void");

    lowered_vm << "push constant 0" << endl;
    lowered_vm << "return" << endl;
  }
  else
  {
    if (pExpression)
    {
      if ((pSubDescr->type == TokenValue_t::J_METHOD) &&
          get_if<Symbol::BasicType_t>(&pSubDescr->return_type) &&
          (get<Symbol::BasicType_t>(pSubDescr->return_type) ==
           Symbol::BasicType_t::T_VOID))
      {
        throw SemanticException("Method declared void but returning non-void");
      }

      lower_expression(pExpression);
    }
    else
    {
      // Only Methods can return nothing
      assert(pSubDescr->type == TokenValue_t::J_METHOD);
      lowered_vm << "push constant 0" << endl;
    }
    lowered_vm << "return" << endl;
  }
}

void VmWriter::lower_statement_list(
    const ParseTreeNonTerminal* pStatementListNode)
{
  assert(pStatementListNode->type == ParseTreeNodeType_t::P_STATEMENT_LIST);

  auto childNodes = pStatementListNode->get_child_nodes();

  for (auto child : childNodes)
  {
    auto pS = dynamic_cast<const ParseTreeNonTerminal*>(&(*child));

    switch ((*child).type)
    {
      case ParseTreeNodeType_t::P_DO_STATEMENT:
        lower_do_statement(pS);
        break;
      case ParseTreeNodeType_t::P_IF_STATEMENT:
        lower_if_statement(pS);
        break;
      case ParseTreeNodeType_t::P_LET_STATEMENT:
        lower_let_statement(pS);
        break;
      case ParseTreeNodeType_t::P_RETURN_STATEMENT:
        lower_return_statement(pS);
        break;
      case ParseTreeNodeType_t::P_WHILE_STATEMENT:
        lower_while_statement(pS);
        break;
      default:
        assert(0 && "fallthrough");
        throw SemanticException("Unexpected node type");
    }
  }
}

void VmWriter::lower_subroutine(const ParseTreeNonTerminal* pSubDeclBlockNode)
{
  pSubDescr = make_unique<SubroutineDescr>(*this, pSubDeclBlockNode);

  symbol_table.reset(Symbol::ScopeLevel_t::SUBROUTINE);
  symbol_table.pSubroutineDescr = &(*pSubDescr);

  create_subroutine_symtable(pSubDeclBlockNode);

  if ((pSubDescr->type == TokenValue_t::J_CONSTRUCTOR) &&
      (!get_if<Symbol::ClassType_t>(&pSubDescr->return_type)))
  {
    throw SemanticException("constructor not returning a class type");
  }

  lowered_vm << "function " << class_name << "." << pSubDescr->name;
  lowered_vm << " " << get_symbol_table().num_locals() << endl;

  // For class constructors, allocate class object and save to pointer 0
  if (pSubDescr->type == TokenValue_t::J_CONSTRUCTOR)
  {
    int num_fields = get_symbol_table().num_fields();
    lowered_vm << "push constant " << num_fields << endl;
    lowered_vm << "call Memory.alloc 1" << endl;
    lowered_vm << "pop pointer 0" << endl;
  }

  // For class methods, get the THIS pointer passed in as argument 0
  // and set up POINTER 0
  if (const Symbol* this_var = symbol_table.find_symbol("this"))
  {
    lowered_vm << "push argument " << this_var->var_index << endl;
    lowered_vm << "pop pointer 0" << endl;
  }

  auto pStatementListNode = find_first_nonterm_node(
      ParseTreeNodeType_t::P_STATEMENT_LIST, pSubDescr->pBody);

  auto childNodes = pStatementListNode->get_child_nodes();

  if (childNodes.size() == 0) throw SemanticException("empty subroutine body");

  lower_statement_list(pStatementListNode);
}

void VmWriter::lower_op(const ParseTreeTerminal* pOp)
{
  string vm_op;

  // raise(SIGTRAP);

  switch (pOp->token.value_enum)
  {
    case TokenValue_t::J_PLUS:
      vm_op = "add";
      break;
    case TokenValue_t::J_MINUS:
      vm_op = "sub";
      break;
    case TokenValue_t::J_ASTERISK:
      vm_op = "call Math.multiply 2";
      break;
    case TokenValue_t::J_DIVIDE:
      vm_op = "call Math.divide 2";
      break;
    case TokenValue_t::J_AMPERSAND:
      vm_op = "and";
      break;
    case TokenValue_t::J_VBAR:
      vm_op = "or";
      break;
    case TokenValue_t::J_LESS_THAN:
      vm_op = "lt";
      break;
    case TokenValue_t::J_GREATER_THAN:
      vm_op = "gt";
      break;
    case TokenValue_t::J_EQUAL:
      vm_op = "eq";
      break;
    default:
      throw SemanticException("Unexpected op fallthrough");
  }

  lowered_vm << vm_op << endl;
}

void VmWriter::lower_term(const ParseTreeNonTerminal* pTerm)
{
  vector<const ParseTreeNode*> child_nodes;

  for (auto node : pTerm->get_child_nodes()) child_nodes.push_back(&(*node));

  if (child_nodes.size() == 1)
  {
    if (auto pT = dynamic_cast<const ParseTreeTerminal*>(child_nodes[0]))
    {
      if (pT->type == ParseTreeNodeType_t::P_INTEGER_CONSTANT)
      {
        string int_string = pT->token.value_str;
        int int_value = -1;
        try
        {
          int_value = stoi(int_string);
        } catch (...)
        {
          throw SemanticException("Expecting integer value 0..32767");
        }

        if (int_value > 0x7fff)
          throw SemanticException("Integer constant out of range.  **" +
                                  int_string + "** > 32767");

        if (int_value < 0)
          throw SemanticException("Unexpected encountered negative number");

        lowered_vm << "push constant " << int_value << endl;
      }
      else if (pT->type == ParseTreeNodeType_t::P_KEYWORD_CONSTANT)
      {
        switch (pT->token.value_enum)
        {
          case TokenValue_t::J_TRUE:
            // TRUE is defined as 0xFFFF (i.e. -1) in the VM
            lowered_vm << "push constant 0" << endl;
            lowered_vm << "not" << endl;
            break;
          case TokenValue_t::J_FALSE:
          case TokenValue_t::J_NULL:
            lowered_vm << "push constant 0" << endl;
            break;
          case TokenValue_t::J_THIS:
            if (pSubDescr->type == TokenValue_t::J_FUNCTION)
              throw SemanticException(
                  "Reference to 'this' not permitted in a function");
            lowered_vm << "push pointer 0" << endl;
            break;
            // TODO: okay in constructor and method
            // TODO: need test for this
          default:
            throw SemanticException("Unexpected term fallthrough");
        }
      }
      else if (pT->type == ParseTreeNodeType_t::P_SCALAR_VAR)
      {
        string varname = get_term_node_str(pT);
        const Symbol* rhs = symbol_table.find_symbol(varname);

        if (!rhs) throw SemanticException("Variable not found: " + varname);

        if (auto pClassSC =
                get_if<Symbol::ClassStorageClass_t>(&rhs->storage_class))
        {
          switch (*pClassSC)
          {
            case Symbol::ClassStorageClass_t::S_STATIC:
              lowered_vm << "push static ";
              break;
            case Symbol::ClassStorageClass_t::S_FIELD:
              lowered_vm << "push this ";
              break;
          }
        }
        else if (auto pSubroutineSC = get_if<Symbol::SubroutineStorageClass_t>(
                     &rhs->storage_class))
        {
          switch (*pSubroutineSC)
          {
            case Symbol::SubroutineStorageClass_t::S_ARGUMENT:
              lowered_vm << "push argument ";
              break;
            case Symbol::SubroutineStorageClass_t::S_LOCAL:
              lowered_vm << "push local ";
              break;
          }
        }

        lowered_vm << rhs->var_index << endl;
      }
      else if (pT->type == ParseTreeNodeType_t::P_STRING_CONSTANT)
      {
        const string& s(get_term_node_str(pT));
        lowered_vm << "push constant " << s.length() << endl;
        lowered_vm << "call String.new 1" << endl;
        for (auto ch : s)
        {
          lowered_vm << "push constant " << (int)ch << endl;
          lowered_vm << "call String.appendChar 2" << endl;
        }
      }
      else
      {
        throw SemanticException("TODO/TERM: unhandled terminal case");
      }
    }
    else if (auto pN =
                 dynamic_cast<const ParseTreeNonTerminal*>(child_nodes[0]))
    {
      if (pN->type == ParseTreeNodeType_t::P_SUBROUTINE_CALL)
      {
        lower_subroutine_call(pN);
      }
      else if (pN->type == ParseTreeNodeType_t::P_ARRAY_BINDING)
      {
        string varname = get_term_node_str(
            find_first_term_node(ParseTreeNodeType_t::P_ARRAY_VAR, pN));

        const Symbol* lhs = symbol_table.find_symbol(varname);
        if (!lhs) throw SemanticException("Variable not found: " + varname);

        auto pArrayExpr =
            find_first_nonterm_node(ParseTreeNodeType_t::P_EXPRESSION, pN);
        assert(pArrayExpr);
        lower_expression(pArrayExpr);

        // Calculate array address + offset
        if (auto pClassSC =
                get_if<Symbol::ClassStorageClass_t>(&lhs->storage_class))
        {
          switch (*pClassSC)
          {
            case Symbol::ClassStorageClass_t::S_STATIC:
              lowered_vm << "push static ";
              break;
            case Symbol::ClassStorageClass_t::S_FIELD:
              lowered_vm << "push this ";
              break;
          }
        }
        else if (auto pSubroutineSC = get_if<Symbol::SubroutineStorageClass_t>(
                     &lhs->storage_class))
        {
          switch (*pSubroutineSC)
          {
            case Symbol::SubroutineStorageClass_t::S_ARGUMENT:
              lowered_vm << "push argument ";
              break;
            case Symbol::SubroutineStorageClass_t::S_LOCAL:
              lowered_vm << "push local ";
              break;
          }
        }

        lowered_vm << lhs->var_index << endl;
        lowered_vm << "add" << endl;

        lowered_vm << "pop pointer 1" << endl;

        lowered_vm << "push that 0" << endl;
      }
      else
      {
        throw SemanticException("TODO/TERM: unhandled non-terminals");
      }
    }
  }
  else if (child_nodes.size() == 2)
  {
    if (child_nodes[0]->type == ParseTreeNodeType_t::P_UNARY_OP &&
        child_nodes[1]->type == ParseTreeNodeType_t::P_TERM)
    {
      auto pO = dynamic_cast<const ParseTreeTerminal*>(child_nodes[0]);
      auto pT = dynamic_cast<const ParseTreeNonTerminal*>(child_nodes[1]);
      lower_term(pT);
      switch (pO->token.value_enum)
      {
        case (TokenValue_t::J_MINUS):
          lowered_vm << "neg" << endl;
          break;
        case (TokenValue_t::J_TILDE):
          lowered_vm << "not" << endl;
          break;
        default:
          throw SemanticException("fallthrough unary op");
      }
    }
  }
  else if (child_nodes.size() == 3)
  {
    if (child_nodes[0]->type == ParseTreeNodeType_t::P_DELIMITER &&
        child_nodes[1]->type == ParseTreeNodeType_t::P_EXPRESSION)
    {
      auto pE = dynamic_cast<const ParseTreeNonTerminal*>(child_nodes[1]);
      lower_expression(pE);
    }
    else
    {
      throw SemanticException("TODO/TERM: fallthrough 3-term");
    }
  }
  else
  {
    throw SemanticException("TODO/TERM: fallthrough N-term");
  }
}

void VmWriter::lower_expression(const ParseTreeNonTerminal* pExpression)
{
  std::queue<const ParseTreeNode*> worklist;
  // raise(SIGTRAP);

  auto N = pExpression->get_child_nodes();

  // <expression> ::= <term> {<op> <term>}*
  if (decltype(N)::iterator it = N.begin(); it != N.end())
  {
    const ParseTreeNode* pN = &(*(*it));
    assert(pN->type == ParseTreeNodeType_t::P_TERM);
    // enqueue <term> #1
    worklist.push(pN);

    while (++it != N.end())
    {
      const ParseTreeNode* pON = &(*(*it));
      assert(pON->type == ParseTreeNodeType_t::P_OP);

      ++it;
      assert(it != N.end());

      pN = &(*(*it));
      assert(pN->type == ParseTreeNodeType_t::P_TERM);
      // enqueue <term> #2
      worklist.push(pN);

      // enqueue <op>
      worklist.push(pON);
    }
  }

  while (!worklist.empty())
  {
    auto pN = worklist.front();
    worklist.pop();

    if (auto pT = dynamic_cast<const ParseTreeNonTerminal*>(pN))
    {
      lower_term(pT);
    }
    else if (auto pO = dynamic_cast<const ParseTreeTerminal*>(pN))
    {
      lower_op(pO);
    }
  }
}

void VmWriter::lower_subroutine_call(
    const ParseTreeNonTerminal* pSubroutineCall)
{
  stringstream call_site_name;

  // Determine call-site name first.  If this is in the symbol table, it must
  // be a class variable.  If it has a '.' in the name, it is a function or
  // constructor call.  Otherwise, it is a method call.
  auto pCallSite = find_first_nonterm_node(
      ParseTreeNodeType_t::P_SUBROUTINE_CALL_SITE_BINDING, pSubroutineCall);
  assert(pCallSite);
  auto pCallVar =
      find_first_term_node(ParseTreeNodeType_t::P_CLASS_OR_VAR_NAME, pCallSite);

  size_t num_arguments = 0;

  if (pCallVar)
  {
    if (const Symbol* this_var =
            symbol_table.find_symbol(pCallVar->token.value_str))
    {
      if (auto pClassSC =
              get_if<Symbol::ClassStorageClass_t>(&this_var->storage_class))
      {
        switch (*pClassSC)
        {
          case Symbol::ClassStorageClass_t::S_STATIC:
            lowered_vm << "push static ";
            break;
          case Symbol::ClassStorageClass_t::S_FIELD:
            lowered_vm << "push this ";
            break;
        }

        auto pTypeName = get_if<Symbol::ClassType_t>(&this_var->type);
        assert(pTypeName);

        call_site_name << *pTypeName << ".";
      }
      else if (auto pSubroutineSC = get_if<Symbol::SubroutineStorageClass_t>(
                   &this_var->storage_class))
      {
        switch (*pSubroutineSC)
        {
          case Symbol::SubroutineStorageClass_t::S_ARGUMENT:
            lowered_vm << "push argument ";
            break;
          case Symbol::SubroutineStorageClass_t::S_LOCAL:
            lowered_vm << "push local ";
            break;
        }

        auto pTypeName = get_if<Symbol::ClassType_t>(&this_var->type);
        assert(pTypeName);

        call_site_name << *pTypeName << ".";
      }

      lowered_vm << this_var->var_index << endl;
      num_arguments++;
    }
    else
    {
      call_site_name << get_term_node_str(pCallVar) << ".";
    }
  }
  else if (const Symbol* this_var = symbol_table.find_symbol("this"))
  {
    lowered_vm << "push pointer 0" << endl;
    num_arguments++;

    auto pThisVarTypeName = get_if<Symbol::ClassType_t>(&this_var->type);
    assert(pThisVarTypeName);
    call_site_name << *pThisVarTypeName << ".";
  }
  else
  {
    // Class method call from constructor
    assert(pSubDescr->type == TokenValue_t::J_CONSTRUCTOR);

    // provide 'this' pointer to internal method call
    lowered_vm << "push pointer 0" << endl;
    num_arguments++;

    call_site_name << class_name << ".";
  }

  auto pSubName =
      find_first_term_node(ParseTreeNodeType_t::P_SUBROUTINE_NAME, pCallSite);

  call_site_name << pSubName->token.value_str;

  auto pExpressionList = find_first_nonterm_node(
      ParseTreeNodeType_t::P_EXPRESSION_LIST, pSubroutineCall);

  for (auto pN : pExpressionList->get_child_nodes())
  {
    if (auto pE = dynamic_cast<const ParseTreeNonTerminal*>(&(*pN)))
    {
      num_arguments++;
      assert(pE->type == ParseTreeNodeType_t::P_EXPRESSION);
      if (pE) lower_expression(pE);
    }
  }

  lowered_vm << "call " << call_site_name.str() << " ";
  lowered_vm << num_arguments << endl;
}

void VmWriter::lower_if_statement(const ParseTreeNonTerminal* pS)
{
  // IF not EXPR GOTO FALSE-PART
  //    TRUE_STATEMENTS
  //    GOTO IF-END
  // FALSE-PART
  //    FALSE_STATEMENTS
  // IF-END
  auto pE = find_first_nonterm_node(ParseTreeNodeType_t::P_EXPRESSION, pS);
  lower_expression(pE);

  lowered_vm << "not" << endl;

  std::vector<std::shared_ptr<ParseTreeNode>> if_nodes;
  for (auto child_node : pS->get_child_nodes()) if_nodes.push_back(child_node);

  bool has_else = false;

  // raise(SIGTRAP);

  if (if_nodes.size() >= 11) has_else = true;

  stringstream control_label;

  // CLASS-NAME.FUNCTION-NAME
  control_label << class_name << '.';
  control_label << pSubDescr->name;

  const auto ID = pSubDescr->structured_control_id++;

  assert(if_nodes[5]->type == ParseTreeNodeType_t::P_STATEMENT_LIST);
  auto pTrueStatementBlock =
      dynamic_cast<const ParseTreeNonTerminal*>(&(*if_nodes[5]));

  if (has_else)
  {
    lowered_vm << "if-goto " << control_label.str() << ".";
    lowered_vm << ID << ".IF_FALSE" << endl;
    lower_statement_list(pTrueStatementBlock);
    lowered_vm << "goto " << control_label.str() << ".";
    lowered_vm << ID << ".IF_END" << endl;

    assert(if_nodes[9]->type == ParseTreeNodeType_t::P_STATEMENT_LIST);
    auto pFalseStatementBlock =
        dynamic_cast<const ParseTreeNonTerminal*>(&(*if_nodes[9]));

    lowered_vm << "label " << control_label.str() << ".";
    lowered_vm << ID << ".IF_FALSE" << endl;
    lower_statement_list(pFalseStatementBlock);
    lowered_vm << "label " << control_label.str() << ".";
    lowered_vm << ID << ".IF_END" << endl;
  }
  else
  {
    lowered_vm << "if-goto " << control_label.str() << ".";
    lowered_vm << ID << ".IF_END" << endl;
    lower_statement_list(pTrueStatementBlock);
    lowered_vm << "label " << control_label.str() << ".";
    lowered_vm << ID << ".IF_END" << endl;
  }
}

void VmWriter::lower_while_statement(const ParseTreeNonTerminal* pS)
{
  // WHILE-BEGIN
  // IF-GOTO not EXPR GOTO WHILE-END
  //    WHILE_STATEMENT
  // GOTO WHILE_BEGIN
  // WHILE-END
  auto pE = find_first_nonterm_node(ParseTreeNodeType_t::P_EXPRESSION, pS);

  std::vector<std::shared_ptr<ParseTreeNode>> while_nodes;
  for (auto child_node : pS->get_child_nodes())
    while_nodes.push_back(child_node);

  stringstream control_label;

  // CLASS-NAME.FUNCTION-NAME
  control_label << class_name << '.';
  control_label << pSubDescr->name;

  const auto ID = pSubDescr->structured_control_id++;

  auto pStatementBlock =
      dynamic_cast<const ParseTreeNonTerminal*>(&(*while_nodes[5]));

  lowered_vm << "label " << control_label.str() << ".";
  lowered_vm << ID << ".WHILE_BEGIN" << endl;

  lower_expression(pE);
  lowered_vm << "not" << endl;

  lowered_vm << "if-goto " << control_label.str() << ".";
  lowered_vm << ID << ".WHILE_END" << endl;

  lower_statement_list(pStatementBlock);

  lowered_vm << "goto " << control_label.str() << ".";
  lowered_vm << ID << ".WHILE_BEGIN" << endl;

  lowered_vm << "label " << control_label.str() << ".";
  lowered_vm << ID << ".WHILE_END" << endl;
}

void VmWriter::lower_let_statement(const ParseTreeNonTerminal* pS)
{
  auto pE = find_first_nonterm_node(ParseTreeNodeType_t::P_EXPRESSION, pS);
  assert(pE);
  lower_expression(pE);

  auto pScalarVar = find_first_term_node(ParseTreeNodeType_t::P_SCALAR_VAR, pS);
  auto pArrayBinding =
      find_first_nonterm_node(ParseTreeNodeType_t::P_ARRAY_BINDING, pS);

  string varname;

  if (pScalarVar)
    varname = get_term_node_str(pScalarVar);
  else if (pArrayBinding)
    varname = get_term_node_str(
        find_first_term_node(ParseTreeNodeType_t::P_ARRAY_VAR, pArrayBinding));
  else
    assert(0 && "Fall through");

  const Symbol* lhs = symbol_table.find_symbol(varname);
  if (!lhs) throw SemanticException("Variable not found: " + varname);

  if (pScalarVar)
  {
    if (auto pClassSC =
            get_if<Symbol::ClassStorageClass_t>(&lhs->storage_class))
    {
      switch (*pClassSC)
      {
        case Symbol::ClassStorageClass_t::S_STATIC:
          lowered_vm << "pop static ";
          break;
        case Symbol::ClassStorageClass_t::S_FIELD:
          lowered_vm << "pop this ";
          break;
      }
    }
    else if (auto pSubroutineSC =
                 get_if<Symbol::SubroutineStorageClass_t>(&lhs->storage_class))
    {
      switch (*pSubroutineSC)
      {
        case Symbol::SubroutineStorageClass_t::S_ARGUMENT:
          lowered_vm << "pop argument ";
          break;
        case Symbol::SubroutineStorageClass_t::S_LOCAL:
          lowered_vm << "pop local ";
          break;
      }
    }

    lowered_vm << lhs->var_index << endl;
  }
  else
  {
    auto pArrayExpr = find_first_nonterm_node(ParseTreeNodeType_t::P_EXPRESSION,
                                              pArrayBinding);
    assert(pArrayExpr);
    lower_expression(pArrayExpr);

    // Calculate array address + offset
    if (auto pClassSC =
            get_if<Symbol::ClassStorageClass_t>(&lhs->storage_class))
    {
      switch (*pClassSC)
      {
        case Symbol::ClassStorageClass_t::S_STATIC:
          lowered_vm << "push static ";
          break;
        case Symbol::ClassStorageClass_t::S_FIELD:
          lowered_vm << "push this ";
          break;
      }
    }
    else if (auto pSubroutineSC =
                 get_if<Symbol::SubroutineStorageClass_t>(&lhs->storage_class))
    {
      switch (*pSubroutineSC)
      {
        case Symbol::SubroutineStorageClass_t::S_ARGUMENT:
          lowered_vm << "push argument ";
          break;
        case Symbol::SubroutineStorageClass_t::S_LOCAL:
          lowered_vm << "push local ";
          break;
      }
    }

    lowered_vm << lhs->var_index << endl;
    lowered_vm << "add" << endl;

    lowered_vm << "pop pointer 1" << endl;

    lowered_vm << "pop that 0" << endl;
  }
}

void VmWriter::lower_do_statement(const ParseTreeNonTerminal* pDoStatement)
{
  auto pSubroutineCall = find_first_nonterm_node(
      ParseTreeNodeType_t::P_SUBROUTINE_CALL, pDoStatement);

  lower_subroutine_call(pSubroutineCall);

  // discare call-ed subroutine's return
  lowered_vm << "pop temp 0" << endl;
}
