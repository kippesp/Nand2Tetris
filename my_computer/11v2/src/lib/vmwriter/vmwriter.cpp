#include "vmwriter/vmwriter.h"

#include "class_descr.h"
#include "semantic_exception.h"

#include <cassert>
#include <iostream>
#include <queue>

using namespace std;

namespace jfcl {

void VmWriter::lower_module()
{
  if (module_root.get().type == AstNodeType_t::N_CLASS_DECL)
  {
    lower_class(module_root);
  }
  else
  {
    throw SemanticException("Expected CLASS declaration as initial node");
  }
}

// Helper function to find and return the value of type T of the the first node
// of type `node_type` among the child nodes of `node`.
template <typename T>
const T& VmWriter::get_ast_node_value(AstNodeCRef node, AstNodeType_t node_type)
{
  AstNodeCRef ast_node = module_ast.find_child_node(node, node_type);

  if (ast_node.get() == EmptyNodeRef.get())
  {
    throw SemanticException("Expected node type not found in child nodes");
  }

  if (const auto s_ptr(get_if<T>(&ast_node.get().value)); s_ptr)
  {
    return *s_ptr;
  }

  throw SemanticException("Found child node's value is not of correct type");
}

template <typename T>
const T& VmWriter::get_ast_node_value(AstNodeCRef node)
{
  if (node.get() == EmptyNodeRef.get())
  {
    throw SemanticException("Expected node type not found in child nodes");
  }

  if (const auto s_ptr(get_if<T>(&node.get().value)); s_ptr)
  {
    return *s_ptr;
  }

  stringstream ss;
  ss << "Found child node's value is not of correct type:\n";
  ss << node.get();

  throw SemanticException(ss.str());
}

optional<VmWriter::SymbolLoweringLocations_t> VmWriter::get_symbol_alloc_info(
    SubroutineDescr& subroutine_descr, const AstNode& node)
{
  auto& var_name = get_ast_node_value<string>(node);

  return get_symbol_alloc_info(subroutine_descr, var_name);
}

optional<VmWriter::SymbolLoweringLocations_t> VmWriter::get_symbol_alloc_info(
    SubroutineDescr& subroutine_descr, const string& symbol_name)
{
  SymbolLoweringLocations_t rvalue;

  if (auto bind_var = subroutine_descr.find_symbol(symbol_name);
      bind_var.has_value())
  {
    switch (bind_var->scope_level)
    {
      case SymbolTable::ScopeLevel_t::CLASS:
        switch (bind_var->storage_class)
        {
          case SymbolTable::StorageClass_t::S_STATIC:
            rvalue.stack_name = "static";
            break;
          case SymbolTable::StorageClass_t::S_FIELD:
            rvalue.stack_name = "this";
            break;
          case SymbolTable::StorageClass_t::S_ARGUMENT:
          case SymbolTable::StorageClass_t::S_LOCAL:
            assert(0 && "invalid storage class for CLASS");
            throw SemanticException("invalid storage class for CLASS");
        }

        break;
      case SymbolTable::ScopeLevel_t::SUBROUTINE:
        switch (bind_var->storage_class)
        {
          case SymbolTable::StorageClass_t::S_ARGUMENT:
            rvalue.stack_name = "argument";
            break;
          case SymbolTable::StorageClass_t::S_LOCAL:
            rvalue.stack_name = "local";
            break;
          case SymbolTable::StorageClass_t::S_STATIC:
          case SymbolTable::StorageClass_t::S_FIELD:
            assert(0 && "invalid storage class for SUBROUTINE");
            throw SemanticException("invalid storage class for SUBROUTINE");
        }

        break;
    }

    rvalue.variable_type = bind_var->variable_type;
    rvalue.scope_level = bind_var->scope_level;
    rvalue.symbol_index = bind_var->index;

    return rvalue;
  }

  return std::nullopt;
}

void VmWriter::lower_class(AstNodeCRef root)
{
  auto& class_name = get_ast_node_value<std::string>(root);
  ClassDescr& class_descr = program.add_class(class_name, root);

  for (auto& node : root.get().get_child_nodes())
  {
    auto node_type = node.get().type;

    // Add any class variables to the class symbol table
    if (node_type == AstNodeType_t::N_CLASS_VARIABLES)
    {
      for (auto& var_node : node.get().get_child_nodes())
      {
        assert(node.get().num_child_nodes() > 0);

        if (var_node.get().type != AstNodeType_t::N_VARIABLE_DECL)
        {
          throw SemanticException("Expected class variable decl node");
        }

        auto& variable_name = get_ast_node_value<std::string>(var_node);
        auto& variable_scope = get_ast_node_value<std::string>(
            var_node, AstNodeType_t::N_CLASS_VARIABLE_SCOPE);
        auto& variable_type = get_ast_node_value<std::string>(
            var_node, AstNodeType_t::N_VARIABLE_TYPE);

        class_descr.add_symbol(variable_name, variable_scope, variable_type);
      }
    }
    else if ((node_type == AstNodeType_t::N_FUNCTION_DECL) ||
             (node_type == AstNodeType_t::N_METHOD_DECL) ||
             (node_type == AstNodeType_t::N_CONSTRUCTOR_DECL))
    {
      lower_subroutine(class_descr, node.get());
    }
    else
    {
      cout << node.get() << endl;
      throw SemanticException("Unexpected node");
    }
  }
}

void VmWriter::lower_statement_block(SubroutineDescr& subroutine_descr,
                                     const AstNode& root)
{
  for (auto& node : root.get_child_nodes())
  {
    if (node.get().type == AstNodeType_t::N_RETURN_STATEMENT)
    {
      lower_return_statement(subroutine_descr, node);
    }
    else if (node.get().type == AstNodeType_t::N_LET_STATEMENT)
    {
        lower_let_statement(subroutine_descr, node);
    }
    else if (node.get().type == AstNodeType_t::N_DO_STATEMENT)
    {
      assert(node.get().num_child_nodes() == 1);
      auto& call_site_parent = node.get().get_child_nodes()[0].get();

      if (call_site_parent.type != AstNodeType_t::N_SUBROUTINE_CALL)
      {
        throw SemanticException("Subroutine call expected following DO");
      }

      lower_subroutine_call(subroutine_descr, call_site_parent);
      // throw away call-ed subroutine's return value
      lowered_vm << "pop temp 0" << endl;
    }
    else if (node.get().type == AstNodeType_t::N_WHILE_STATEMENT)
    {
      lower_while_statement(subroutine_descr, node);
    }
    else if (node.get().type == AstNodeType_t::N_IF_STATEMENT)
    {
      lower_if_statement(subroutine_descr, node);
    }
    else
    {
      throw SemanticException("fallthrough");
    }
  }
}

void VmWriter::lower_subroutine(ClassDescr& class_descr, const AstNode& root)
{
  auto& subroutine_name = get_ast_node_value<std::string>(root);
  const AstNode& DescrNode =
      module_ast.find_child_node(root, AstNodeType_t::N_SUBROUTINE_DESCR).get();
  auto return_type = SymbolTable::variable_type_from_string(
      get_ast_node_value<string>(DescrNode, AstNodeType_t::N_RETURN_TYPE));

  SubroutineDescr& subroutine_descr =
      class_descr.add_subroutine(subroutine_name, return_type, root).get();

  // For class methods, add the implicit argument "this" representing the class
  if (root.type == AstNodeType_t::N_METHOD_DECL)
  {
    subroutine_descr.add_symbol("this", "argument", class_descr.get_name());
  }

  // Helper function to add to subroutine's symbol table
  auto add_symbol = [&](AstNodeType_t node_type) -> void {
    if (const AstNode& VariablesNode =
            module_ast.find_child_node(DescrNode, node_type).get();
        VariablesNode != EmptyNodeRef.get())
    {
      for (auto& node : VariablesNode.get_child_nodes())
      {
        if (node.get().type != AstNodeType_t::N_VARIABLE_DECL)
        {
          throw SemanticException("Expected subroutine input param decl node");
        }

        auto& variable_name = get_ast_node_value<std::string>(node);
        auto& variable_type = get_ast_node_value<std::string>(
            node, AstNodeType_t::N_VARIABLE_TYPE);

        subroutine_descr.add_symbol(
            variable_name,
            (node_type == AstNodeType_t::N_INPUT_PARAMETERS) ? "argument"
                                                             : "local",
            variable_type);
      }
    }
  };

  // Add any argument variables to the symbol table
  add_symbol(AstNodeType_t::N_INPUT_PARAMETERS);

  // Add any subroutine local variables to the symbol table
  add_symbol(AstNodeType_t::N_LOCAL_VARIABLES);

  lowered_vm << "function " << class_descr.get_name() << "." << subroutine_name;
  lowered_vm << " " << subroutine_descr.num_locals() << endl;

  // For class constructors, allocate class object and save to pointer 0
  if (root.type == AstNodeType_t::N_CONSTRUCTOR_DECL)
  {
    lowered_vm << "push constant " << subroutine_descr.num_fields() << endl;
    lowered_vm << "call Memory.alloc 1" << endl;
    lowered_vm << "pop pointer 0" << endl;
  }

  // For class methods, get the THIS pointer passed in as argument 0
  // and set up POINTER 0
  if (auto this_var = subroutine_descr.find_symbol("this");
      this_var.has_value())
  {
    Symbol& symbol = *this_var;
    lowered_vm << "push argument " << symbol.index << endl;
    lowered_vm << "pop pointer 0" << endl;
  }

  AstNodeCRef BodyNode =
      module_ast.find_child_node(root, AstNodeType_t::N_SUBROUTINE_BODY);

  AstNodeCRef StatementBlockNode =
      module_ast.find_child_node(BodyNode, AstNodeType_t::N_STATEMENT_BLOCK);

  lower_statement_block(subroutine_descr, StatementBlockNode.get());
}

string VmWriter::lower_expression(SubroutineDescr& subroutine_descr,
                                  const AstNode& expression_root)
{
  queue<AstNodeCRef> worklist;

  function<void(AstNodeCRef)> visit_fn;

  visit_fn = [&](AstNodeCRef new_node) {
    if ((new_node.get().num_child_nodes() == 0) ||
        (new_node.get().type == AstNodeType_t::N_SUBROUTINE_CALL))
    {
      worklist.push(new_node);
    }
    else
    {
      for (auto& next_node : new_node.get().get_child_nodes())
      {
        visit_fn(next_node);
      }

      worklist.push(new_node);
    }
  };

  // build DFS node queue the expression tree
  visit_fn(expression_root);

  while (worklist.size() > 0)
  {
    auto& node = worklist.front();
    auto& node_type = node.get().type;

    worklist.pop();

    if (node_type == AstNodeType_t::N_INTEGER_CONSTANT)
    {
      auto int_value = get_ast_node_value<int>(node);

      if (int_value > 0x7fff)
      {
        stringstream ss;
        ss << "Integer constant out of range.  **" << int_value << "** > 32767";
        throw SemanticException(ss.str());
      }

      if (int_value < 0)
        throw SemanticException("Unexpected encountered negative number");

      lowered_vm << "push constant " << int_value << endl;
    }
    else if (node_type == AstNodeType_t::N_THIS_KEYWORD)
    {
      if (subroutine_descr.get_root().type == AstNodeType_t::N_FUNCTION_DECL)
      {
        throw SemanticException("'this' keyword not permitted in functions");
      }

      lowered_vm << "push pointer 0" << endl;
    }
    else if (node_type == AstNodeType_t::N_VARIABLE_NAME)
    {
      lower_var(subroutine_descr, node);
    }
    else if (node_type == AstNodeType_t::N_SUBROUTINE_CALL)
    {
      lower_subroutine_call(subroutine_descr, node);
    }
    else if (node_type == AstNodeType_t::N_SUBSCRIPTED_VARIABLE_NAME)
    {
      lower_var(subroutine_descr, node);

      lowered_vm << "add" << endl;
      lowered_vm << "pop pointer 1" << endl;
      lowered_vm << "push that 0" << endl;
    }
    else if (node_type == AstNodeType_t::N_STRING_CONSTANT)
    {
      auto& str = get_ast_node_value<string>(node);

      lowered_vm << "push constant " << str.length() << endl;
      lowered_vm << "call String.new 1" << endl;
      for (auto ch : str)
      {
        lowered_vm << "push constant " << static_cast<int>(ch) << endl;
        lowered_vm << "call String.appendChar 2" << endl;
      }
    }
    // handle operators
    else
    {
      string operator_vm;

      if (node_type == AstNodeType_t::N_OP_MULTIPLY)
      {
        lowered_vm << "call Math.multiply 2" << endl;
      }
      else if (node_type == AstNodeType_t::N_OP_DIVIDE)
      {
        lowered_vm << "call Math.divide 2" << endl;
      }
      else if (node_type == AstNodeType_t::N_OP_ADD)
      {
        lowered_vm << "add" << endl;
      }
      else if (node_type == AstNodeType_t::N_OP_SUBTRACT)
      {
        lowered_vm << "sub" << endl;
      }
      else if (node_type == AstNodeType_t::N_OP_LOGICAL_EQUALS)
      {
        lowered_vm << "eq" << endl;
      }
      else if (node_type == AstNodeType_t::N_OP_LOGICAL_GT)
      {
        lowered_vm << "gt" << endl;
      }
      else if (node_type == AstNodeType_t::N_OP_LOGICAL_LT)
      {
        lowered_vm << "lt" << endl;
      }
      else if (node_type == AstNodeType_t::N_OP_LOGICAL_AND)
      {
        lowered_vm << "and" << endl;
      }
      else if (node_type == AstNodeType_t::N_OP_LOGICAL_OR)
      {
        lowered_vm << "or" << endl;
      }
      else if (node_type == AstNodeType_t::N_OP_PREFIX_NEG)
      {
        lowered_vm << "neg" << endl;
      }
      else if (node_type == AstNodeType_t::N_OP_PREFIX_LOGICAL_NOT)
      {
        lowered_vm << "not" << endl;
      }
      else if (node_type == AstNodeType_t::N_TRUE_KEYWORD)
      {
        lowered_vm << "push constant 0" << endl;
        lowered_vm << "not" << endl;
      }
      else if (node_type == AstNodeType_t::N_FALSE_KEYWORD)
      {
        lowered_vm << "push constant 0" << endl;
      }
      else if (node_type == AstNodeType_t::N_NULL_KEYWORD)
      {
        lowered_vm << "push constant 0" << endl;
      }
      else
      {
        stringstream ss;
        ss << "expression fallthrough:\n";
        ss << node.get();
        throw SemanticException(ss.str());
      }
    }
  }

  return "";
}

void VmWriter::lower_var(SubroutineDescr& subroutine_descr, const AstNode& node)
{
  auto symbol_alloc = get_symbol_alloc_info(subroutine_descr, node);

  if (symbol_alloc.has_value())
  {
    auto& symbol = symbol_alloc.value();

    lowered_vm << "push " << symbol.stack_name << " " << symbol.symbol_index
               << endl;
  }
  else
  {
    auto& bind_var_name = get_ast_node_value<string>(node);
    throw SemanticException("Symbol not found:", bind_var_name);
  }
}

void VmWriter::lower_return_statement(SubroutineDescr& subroutine_descr,
                                      const AstNode& root)
{
  assert((root.num_child_nodes() == 0) || (root.num_child_nodes() == 1));

  auto subroutine_type = subroutine_descr.get_root().type;
  auto subroutine_return_type = subroutine_descr.get_return_type();
  bool is_void_return = false;

  if (auto return_type_ptr =
          get_if<SymbolTable::BasicType_t>(&subroutine_return_type);
      return_type_ptr)
  {
    is_void_return = (*return_type_ptr == SymbolTable::BasicType_t::T_VOID);
  }

  if (is_void_return)
  {
    if ((subroutine_type == AstNodeType_t::N_FUNCTION_DECL) ||
        (subroutine_type == AstNodeType_t::N_METHOD_DECL))
    {
      if (root.num_child_nodes() > 0)
      {
        throw SemanticException("Void subroutine returning non-void");
      }

      lowered_vm << "push constant 0" << endl;
      lowered_vm << "return" << endl;
      return;
    }

    assert(subroutine_type == AstNodeType_t::N_CONSTRUCTOR_DECL);

    throw SemanticException("Constructor declared as void");
  }
  else
  {
    if (root.num_child_nodes() == 0)
    {
      throw SemanticException("Non-void subroutine returning void");
    }

    auto& expression_node = root.get_child_nodes()[0].get();

    lower_expression(subroutine_descr, expression_node);
    lowered_vm << "return" << endl;
  }
}

void VmWriter::lower_let_statement(SubroutineDescr& subroutine_descr,
                                   const AstNode& root)
{
  assert(root.num_child_nodes() == 2);

  auto& lh_bind_node = root.get_child_nodes()[0].get();
  auto& rhs_node = root.get_child_nodes()[1].get();

  lower_expression(subroutine_descr, rhs_node);

  if (lh_bind_node.type == AstNodeType_t::N_VARIABLE_NAME)
  {
    auto sym_locs = get_symbol_alloc_info(subroutine_descr, lh_bind_node);

    if (sym_locs.has_value())
    {
      auto& sym = sym_locs.value();

      lowered_vm << "pop " << sym.stack_name << " " << sym.symbol_index << endl;
    }
    else
    {
      auto& bind_var_name = get_ast_node_value<string>(lh_bind_node);
      throw SemanticException("Symbol not found in LET:", bind_var_name);
    }
  }
  else if (lh_bind_node.type == AstNodeType_t::N_SUBSCRIPTED_VARIABLE_NAME)
  {
    auto& subscript_expression_node = lh_bind_node.get_child_nodes()[0].get();

    lower_expression(subroutine_descr, subscript_expression_node);
    lower_var(subroutine_descr, lh_bind_node);

    lowered_vm << "add" << endl;
    lowered_vm << "pop pointer 1" << endl;
    lowered_vm << "pop that 0" << endl;
  }
  else
  {
    assert(0 && "fallthrough");
  }
}

void VmWriter::lower_while_statement(SubroutineDescr& subroutine_descr,
                                     const AstNode& root)
{
  assert(root.num_child_nodes() == 2);

  const auto LOOP_ID = subroutine_descr.get_next_structured_control_id();

  const auto& expression_node = root.get_child_nodes()[0].get();
  const auto& statement_block_node = root.get_child_nodes()[1].get();

  lowered_vm << "label WHILE_BEGIN_" << LOOP_ID << endl;
  lower_expression(subroutine_descr, expression_node);
  lowered_vm << "not" << endl;
  lowered_vm << "if-goto WHILE_END_" << LOOP_ID << endl;
  lower_statement_block(subroutine_descr, statement_block_node);
  lowered_vm << "goto WHILE_BEGIN_" << LOOP_ID << endl;
  lowered_vm << "label WHILE_END_" << LOOP_ID << endl;
}

void VmWriter::lower_if_statement(SubroutineDescr& subroutine_descr,
                                  const AstNode& root)
{
  assert((root.num_child_nodes() == 2) || (root.num_child_nodes() == 3));

  bool has_else = false;
  if (root.num_child_nodes() == 3)
  {
    has_else = true;
  }

  const auto ID = subroutine_descr.get_next_structured_control_id();

  const auto& expression_node = root.get_child_nodes()[0].get();
  const auto& true_statement_block_node = root.get_child_nodes()[1].get();

  lower_expression(subroutine_descr, expression_node);

  if (has_else)
  {
    const auto& false_statement_block_node = root.get_child_nodes()[2].get();

    lowered_vm << "if-goto IF_TRUE_" << ID << endl;
    lowered_vm << "label IF_FALSE_" << ID << endl;
    lower_statement_block(subroutine_descr, false_statement_block_node);
    lowered_vm << "goto IF_END_" << ID << endl;
    lowered_vm << "label IF_TRUE_" << ID << endl;
    lower_statement_block(subroutine_descr, true_statement_block_node);
    lowered_vm << "label IF_END_" << ID << endl;
  }
  else
  {
    lowered_vm << "if-goto IF_TRUE_" << ID << endl;
    lowered_vm << "goto IF_END_" << ID << endl;
    lowered_vm << "label IF_TRUE_" << ID << endl;
    lower_statement_block(subroutine_descr, true_statement_block_node);
    lowered_vm << "label IF_END_" << ID << endl;
  }
}

void VmWriter::lower_subroutine_call(SubroutineDescr& subroutine_descr,
                                     const AstNode& root)
{
  auto& call_site = root.get_child_nodes()[0].get();
  assert((call_site.type == AstNodeType_t::N_LOCAL_CALL_SITE) ||
         (call_site.type == AstNodeType_t::N_GLOBAL_CALL_SITE));

  stringstream call_site_bind_name;

  // number of arguments the subroutine takes and are pushed to the stack
  size_t call_site_args = 0;

  auto subroutine_type = subroutine_descr.get_root().type;

  // If the symbol table has the 'this' variable, we're in a METHOD
  if (auto this_symbol = subroutine_descr.find_symbol("this");
      this_symbol.has_value())
  {
    assert(subroutine_type == AstNodeType_t::N_METHOD_DECL);

    // LOCAL METHOD CALL
    if (call_site.type == AstNodeType_t::N_LOCAL_CALL_SITE)
    {
      // Handle call: subroutine(), this pointer
      lowered_vm << "push pointer 0" << endl;
      call_site_args++;

      if (auto symbol_type_ptr =
              get_if<SymbolTable::ClassType_t>(&this_symbol->variable_type);
          symbol_type_ptr)
      {
        call_site_bind_name << *symbol_type_ptr << ".";
      }
      else
      {
        assert(0 && "Expected ClassType_t");
      }
    }
    // GLOBAL METHOD CALL
    else if (call_site.type == AstNodeType_t::N_GLOBAL_CALL_SITE)
    {
      // Handle call: local_var.subroutine()
      if (auto symbol_alloc = get_symbol_alloc_info(
              subroutine_descr,
              module_ast
                  .find_child_node(call_site, AstNodeType_t::N_SUBROUTINE_NAME)
                  .get());
          symbol_alloc.has_value())
      {
        assert(0 && "Global Call Site/Method/Var Binding");
      }
      else
      {
        // Handle call: ClassName.subroutine()
        auto& global_bind_name = get_ast_node_value<string>(
            call_site, AstNodeType_t::N_GLOBAL_BIND_NAME);

        // Handle call: local_var.subroutine()
        if (auto symbol_alloc_ =
                get_symbol_alloc_info(subroutine_descr, global_bind_name);
            symbol_alloc_.has_value())
        {
          auto& sym = symbol_alloc_.value();

          call_site_args++;
          lowered_vm << "push " << sym.stack_name << " " << sym.symbol_index
                     << endl;

          if (auto class_type_ptr =
                  get_if<SymbolTable::ClassType_t>(&sym.variable_type);
              class_type_ptr)
          {
            call_site_bind_name << *class_type_ptr << ".";
          }
          else
          {
            assert(0 && "Expected ClassType_t");
          }
        }
        else
        {
          call_site_bind_name << global_bind_name << ".";
        }
      }
    }
    else
    {
      assert(0 && "Unexpected fallthrough");
    }
  }
  // Otherwise, this is either a CONSTRUCTOR or FUNCTION
  else
  {
    assert((subroutine_type == AstNodeType_t::N_CONSTRUCTOR_DECL) ||
           (subroutine_type == AstNodeType_t::N_FUNCTION_DECL));

    // LOCAL METHOD CALL
    if (call_site.type == AstNodeType_t::N_LOCAL_CALL_SITE)
    {
      // Handle call: subroutine()
      lowered_vm << "push pointer 0" << endl;
      call_site_args++;
      call_site_bind_name << subroutine_descr.get_class_name() << ".";
    }
    // GLOBAL METHOD CALL
    else if (call_site.type == AstNodeType_t::N_GLOBAL_CALL_SITE)
    {
      auto& global_bind_name = get_ast_node_value<string>(
          call_site, AstNodeType_t::N_GLOBAL_BIND_NAME);

      // Handle call: local_var.subroutine()
      if (auto symbol_alloc =
              get_symbol_alloc_info(subroutine_descr, global_bind_name);
          symbol_alloc.has_value())
      {
        auto& sym = symbol_alloc.value();

        call_site_args++;
        lowered_vm << "push " << sym.stack_name << " " << sym.symbol_index
                   << endl;

        if (auto class_type_ptr =
                get_if<SymbolTable::ClassType_t>(&sym.variable_type);
            class_type_ptr)
        {
          call_site_bind_name << *class_type_ptr << ".";
        }
        else
        {
          assert(0 && "Expected ClassType_t");
        }
      }
      else
      {
        // Handle call: ClassName.subroutine()
        call_site_bind_name << global_bind_name << ".";
      }
    }
    else
    {
      assert(0 && "Unexpected fallthrough");
    }
  }

  auto& call_site_subroutine_name =
      get_ast_node_value<string>(call_site, AstNodeType_t::N_SUBROUTINE_NAME);
  call_site_bind_name << call_site_subroutine_name;

  // LOWER CALL ARGUMENTS
  const auto& call_args_node =
      module_ast.find_child_node(root, AstNodeType_t::N_CALL_ARGUMENTS).get();
  assert(call_args_node != EmptyNodeRef.get());

  for (auto& node : call_args_node.get_child_nodes())
  {
    lower_expression(subroutine_descr, node.get());
    call_site_args++;
  }

  // LOWER CALL
  lowered_vm << "call " << call_site_bind_name.str() << " " << call_site_args
             << endl;
}

}  // namespace jfcl
