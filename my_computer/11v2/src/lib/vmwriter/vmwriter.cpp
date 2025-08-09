#include "vmwriter/vmwriter.h"

#include "class_descr.h"
#include "parser/ast.h"
#include "semantic_exception.h"
#include "vmwriter/subroutine_descr.h"
#include "vmwriter/symbol_table.h"

#include <cassert>
#include <cstdlib>
#include <functional>
#include <iostream>
#include <optional>
#include <queue>
#include <sstream>
#include <variant>

using namespace std;

#if 0
static bool is_valid_node_reference(
    const std::reference_wrapper<const jfcl::AstNode>& ref)
{
  // Add validation logic
  try
  {
    const jfcl::AstNode& node = ref.get();
    return true;
  }
  catch (...)
  {
    return false;
  }
}
#endif

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
  AstNodeCRef const ast_node = module_ast.find_child_node(node, node_type);

  if (ast_node.get() == EmptyNodeRef.get())
  {
    throw SemanticException("Expected node type not found in child nodes");
  }

  if (const auto* const s_ptr(get_if<T>(&ast_node.get().value)); s_ptr)
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

  if (const auto* const s_ptr(get_if<T>(&node.get().value)); s_ptr)
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
  const auto& var_name = get_ast_node_value<string>(node);

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
  const auto& class_name = get_ast_node_value<std::string>(root);
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

        const auto& variable_name = get_ast_node_value<std::string>(var_node);
        const auto& variable_scope = get_ast_node_value<std::string>(
            var_node, AstNodeType_t::N_CLASS_VARIABLE_SCOPE);
        const auto& variable_type = get_ast_node_value<std::string>(
            var_node, AstNodeType_t::N_VARIABLE_TYPE);

        // Find the variable type node to get line number information
        const AstNode& variable_type_node =
            module_ast.find_child_node(var_node, AstNodeType_t::N_VARIABLE_TYPE)
                .get();

        auto warn_callback = [this,
                              &variable_type_node](const std::string& msg) {
          emit_warning(variable_type_node, msg);
        };

        class_descr.add_symbol(variable_name, variable_scope, variable_type,
                               warn_callback);
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
      cout << node.get() << '\n';
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
      const auto& call_site_parent = node.get().get_child_nodes()[0].get();

      if (call_site_parent.type != AstNodeType_t::N_SUBROUTINE_CALL)
      {
        throw SemanticException("Subroutine call expected following DO");
      }

      lower_subroutine_call(subroutine_descr, call_site_parent);
      // throw away call-ed subroutine's return value
      emit_vm_instruction("pop temp 0");
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
  const auto& subroutine_name = get_ast_node_value<std::string>(root);
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

        const auto& variable_name = get_ast_node_value<std::string>(node);
        const auto& variable_type = get_ast_node_value<std::string>(
            node, AstNodeType_t::N_VARIABLE_TYPE);

        // Find the variable type node to get line number information
        const AstNode& variable_type_node =
            module_ast.find_child_node(node, AstNodeType_t::N_VARIABLE_TYPE)
                .get();

        auto warn_callback = [this,
                              &variable_type_node](const std::string& msg) {
          emit_warning(variable_type_node, msg);
        };

        subroutine_descr.add_symbol(
            variable_name,
            (node_type == AstNodeType_t::N_INPUT_PARAMETERS) ? "argument"
                                                             : "local",
            variable_type, warn_callback);
      }
    }
  };

  // Add any argument variables to the symbol table
  add_symbol(AstNodeType_t::N_INPUT_PARAMETERS);

  // Add any subroutine local variables to the symbol table
  add_symbol(AstNodeType_t::N_LOCAL_VARIABLES);

  stringstream function_decl;
  function_decl << "function " << class_descr.get_name() << "."
                << subroutine_name;
  function_decl << " " << subroutine_descr.num_locals();
  emit_vm_instruction(function_decl.str());

  // For class constructors, allocate class object and save to pointer 0
  if (root.type == AstNodeType_t::N_CONSTRUCTOR_DECL)
  {
    stringstream push_const;
    push_const << "push constant " << subroutine_descr.num_fields();
    emit_vm_instruction(push_const.str());
    emit_vm_instruction("call Memory.alloc 1");
    emit_vm_instruction("pop pointer 0");
  }

  // For class methods, get the THIS pointer passed in as argument 0
  // and set up POINTER 0
  if (auto this_var = subroutine_descr.find_symbol("this");
      this_var.has_value())
  {
    Symbol const& symbol = *this_var;
    stringstream push_arg;
    push_arg << "push argument " << symbol.index;
    emit_vm_instruction(push_arg.str());
    emit_vm_instruction("pop pointer 0");
  }

  AstNodeCRef const BodyNode =
      module_ast.find_child_node(root, AstNodeType_t::N_SUBROUTINE_BODY);

  AstNodeCRef const StatementBlockNode =
      module_ast.find_child_node(BodyNode, AstNodeType_t::N_STATEMENT_BLOCK);

  lower_statement_block(subroutine_descr, StatementBlockNode.get());

  // Validate that non-void functions have return statements
  validate_return_statement(subroutine_descr, StatementBlockNode.get(), root);
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

  while (!worklist.empty())
  {
    auto node = worklist.front();
    const auto& node_type = node.get().type;

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
      {
        throw SemanticException("Unexpected encountered negative number");
      }

      stringstream push_const;
      push_const << "push constant " << int_value;
      emit_vm_instruction(push_const.str());
    }
    else if (node_type == AstNodeType_t::N_THIS_KEYWORD)
    {
      if (subroutine_descr.get_root().type == AstNodeType_t::N_FUNCTION_DECL)
      {
        throw SemanticException("'this' keyword not permitted in functions");
      }

      emit_vm_instruction("push pointer 0");
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

      emit_vm_instruction("add");
      emit_vm_instruction("pop pointer 1");
      emit_vm_instruction("push that 0");
    }
    else if (node_type == AstNodeType_t::N_STRING_CONSTANT)
    {
      const auto& str = get_ast_node_value<string>(node);

      stringstream push_str_len;
      push_str_len << "push constant " << str.length();
      emit_vm_instruction(push_str_len.str());
      emit_vm_instruction("call String.new 1");
      for (auto ch : str)
      {
        stringstream push_char;
        push_char << "push constant " << static_cast<int>(ch);
        emit_vm_instruction(push_char.str());
        emit_vm_instruction("call String.appendChar 2");
      }
    }
    // handle operators
    else
    {
      string const operator_vm;

      if (node_type == AstNodeType_t::N_OP_MULTIPLY)
      {
        validate_binary_operator_types(subroutine_descr, node);
        emit_vm_instruction("call Math.multiply 2");
      }
      else if (node_type == AstNodeType_t::N_OP_DIVIDE)
      {
        validate_binary_operator_types(subroutine_descr, node);
        emit_vm_instruction("call Math.divide 2");
      }
      else if (node_type == AstNodeType_t::N_OP_ADD)
      {
        validate_binary_operator_types(subroutine_descr, node);
        emit_vm_instruction("add");
      }
      else if (node_type == AstNodeType_t::N_OP_SUBTRACT)
      {
        validate_binary_operator_types(subroutine_descr, node);
        emit_vm_instruction("sub");
      }
      else if (node_type == AstNodeType_t::N_OP_LOGICAL_EQUALS)
      {
        validate_binary_operator_types(subroutine_descr, node);
        emit_vm_instruction("eq");
      }
      else if (node_type == AstNodeType_t::N_OP_LOGICAL_GT)
      {
        validate_binary_operator_types(subroutine_descr, node);
        emit_vm_instruction("gt");
      }
      else if (node_type == AstNodeType_t::N_OP_LOGICAL_LT)
      {
        validate_binary_operator_types(subroutine_descr, node);
        emit_vm_instruction("lt");
      }
      else if (node_type == AstNodeType_t::N_OP_BITWISE_AND)
      {
        validate_binary_operator_types(subroutine_descr, node);
        emit_vm_instruction("and");
      }
      else if (node_type == AstNodeType_t::N_OP_BITWISE_OR)
      {
        validate_binary_operator_types(subroutine_descr, node);
        emit_vm_instruction("or");
      }
      else if (node_type == AstNodeType_t::N_OP_PREFIX_NEG)
      {
        emit_vm_instruction("neg");
      }
      else if (node_type == AstNodeType_t::N_OP_PREFIX_BITWISE_NOT)
      {
        emit_vm_instruction("not");
      }
      else if (node_type == AstNodeType_t::N_TRUE_KEYWORD)
      {
        emit_vm_instruction("push constant 0");
        emit_vm_instruction("not");
      }
      else if (node_type == AstNodeType_t::N_FALSE_KEYWORD)
      {
        emit_vm_instruction("push constant 0");
      }
      else if (node_type == AstNodeType_t::N_NULL_KEYWORD)
      {
        emit_vm_instruction("push constant 0");
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

    stringstream push_var;
    push_var << "push " << symbol.stack_name << " " << symbol.symbol_index;
    emit_vm_instruction(push_var.str());
  }
  else
  {
    const auto& bind_var_name = get_ast_node_value<string>(node);
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

  if (auto* return_type_ptr =
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

      emit_vm_instruction("push constant 0");
      emit_vm_instruction("return");
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

    const auto& expression_node = root.get_child_nodes()[0].get();

    lower_expression(subroutine_descr, expression_node);
    emit_vm_instruction("return");
  }
}

void VmWriter::lower_let_statement(SubroutineDescr& subroutine_descr,
                                   const AstNode& root)
{
  assert(root.num_child_nodes() == 2);

  const auto& lh_bind_node = root.get_child_nodes()[0].get();
  const auto& rhs_node = root.get_child_nodes()[1].get();

  // Check for assignment type conversion warnings
  if (lh_bind_node.type == AstNodeType_t::N_VARIABLE_NAME)
  {
    auto sym_locs = get_symbol_alloc_info(subroutine_descr, lh_bind_node);
    if (sym_locs.has_value())
    {
      auto lhs_type = sym_locs->variable_type;
      auto rhs_type = get_expression_type(subroutine_descr, rhs_node);
      check_assignment_type_conversion(lhs_type, rhs_type, rhs_node);
    }
  }

  lower_expression(subroutine_descr, rhs_node);

  if (lh_bind_node.type == AstNodeType_t::N_VARIABLE_NAME)
  {
    auto sym_locs = get_symbol_alloc_info(subroutine_descr, lh_bind_node);

    if (sym_locs.has_value())
    {
      auto& sym = sym_locs.value();

      stringstream pop_var;
      pop_var << "pop " << sym.stack_name << " " << sym.symbol_index;
      emit_vm_instruction(pop_var.str());
    }
    else
    {
      const auto& bind_var_name = get_ast_node_value<string>(lh_bind_node);
      throw SemanticException("Symbol not found in LET:", bind_var_name);
    }
  }
  else if (lh_bind_node.type == AstNodeType_t::N_SUBSCRIPTED_VARIABLE_NAME)
  {
    const auto& subscript_expression_node =
        lh_bind_node.get_child_nodes()[0].get();

    lower_expression(subroutine_descr, subscript_expression_node);
    lower_var(subroutine_descr, lh_bind_node);

    emit_vm_instruction("add");
    emit_vm_instruction("pop pointer 1");
    emit_vm_instruction("pop that 0");
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

  const auto BEGIN_ID = get_next_label_id();
  const auto END_ID = get_next_label_id();

  const auto& expression_node = root.get_child_nodes()[0].get();
  const auto& statement_block_node = root.get_child_nodes()[1].get();

  // Validate that while condition is boolean
  validate_boolean_context(subroutine_descr, expression_node, "while");

  stringstream while_begin_label, while_end_label, if_goto_stmt, goto_stmt;
  while_begin_label << "label WHILE_BEGIN_" << BEGIN_ID;
  while_end_label << "label WHILE_EXIT_" << END_ID;
  if_goto_stmt << "if-goto WHILE_EXIT_" << END_ID;
  goto_stmt << "goto WHILE_BEGIN_" << BEGIN_ID;

  emit_vm_instruction(while_begin_label.str());
  lower_expression(subroutine_descr, expression_node);
  emit_vm_instruction("not");
  emit_vm_instruction(if_goto_stmt.str());
  lower_statement_block(subroutine_descr, statement_block_node);
  emit_vm_instruction(goto_stmt.str());
  emit_vm_instruction(while_end_label.str());
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

  const auto ID = get_next_label_id();

  const auto& expression_node = root.get_child_nodes()[0].get();
  const auto& true_statement_block_node = root.get_child_nodes()[1].get();

  // Validate that if condition is boolean
  validate_boolean_context(subroutine_descr, expression_node, "if");

  lower_expression(subroutine_descr, expression_node);

  if (has_else)
  {
    const auto& false_statement_block_node = root.get_child_nodes()[2].get();

    stringstream if_goto_true, if_false_label, goto_false, if_true_label,
        goto_end, if_end_label;
    if_goto_true << "if-goto IF_TRUE_" << ID;
    if_false_label << "label IF_FALSE_" << ID;
    goto_false << "goto IF_FALSE_" << ID;
    if_true_label << "label IF_TRUE_" << ID;
    goto_end << "goto IF_END_" << ID;
    if_end_label << "label IF_END_" << ID;

    emit_vm_instruction(if_goto_true.str());
    emit_vm_instruction(goto_false.str());
    emit_vm_instruction(if_true_label.str());
    lower_statement_block(subroutine_descr, true_statement_block_node);
    emit_vm_instruction(goto_end.str());
    emit_vm_instruction(if_false_label.str());
    lower_statement_block(subroutine_descr, false_statement_block_node);
    emit_vm_instruction(if_end_label.str());
  }
  else
  {
    stringstream if_goto_true, goto_false, if_true_label, if_false_label;
    if_goto_true << "if-goto IF_TRUE_" << ID;
    goto_false << "goto IF_FALSE_" << ID;
    if_true_label << "label IF_TRUE_" << ID;
    if_false_label << "label IF_FALSE_" << ID;

    emit_vm_instruction(if_goto_true.str());
    emit_vm_instruction(goto_false.str());
    emit_vm_instruction(if_true_label.str());
    lower_statement_block(subroutine_descr, true_statement_block_node);
    emit_vm_instruction(if_false_label.str());
  }
}

void VmWriter::lower_subroutine_call(SubroutineDescr& subroutine_descr,
                                     const AstNode& root)
{
  const auto& call_site = root.get_child_nodes()[0].get();
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
      emit_vm_instruction("push pointer 0");
      call_site_args++;

      if (const auto* symbol_type_ptr =
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
      // Handle call: ClassName.subroutine()
      const auto& global_bind_name = get_ast_node_value<string>(
          call_site, AstNodeType_t::N_GLOBAL_BIND_NAME);

      // Handle call: local_var.subroutine()
      if (auto symbol_alloc_ =
              get_symbol_alloc_info(subroutine_descr, global_bind_name);
          symbol_alloc_.has_value())
      {
        auto& sym = symbol_alloc_.value();

        call_site_args++;
        stringstream push_sym;
        push_sym << "push " << sym.stack_name << " " << sym.symbol_index;
        emit_vm_instruction(push_sym.str());

        if (auto* class_type_ptr =
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
      emit_vm_instruction("push pointer 0");
      call_site_args++;
      call_site_bind_name << subroutine_descr.get_class_name() << ".";
    }
    // GLOBAL METHOD CALL
    else if (call_site.type == AstNodeType_t::N_GLOBAL_CALL_SITE)
    {
      const auto& global_bind_name = get_ast_node_value<string>(
          call_site, AstNodeType_t::N_GLOBAL_BIND_NAME);

      // Handle call: local_var.subroutine()
      if (auto symbol_alloc =
              get_symbol_alloc_info(subroutine_descr, global_bind_name);
          symbol_alloc.has_value())
      {
        auto& sym = symbol_alloc.value();

        call_site_args++;
        stringstream push_sym2;
        push_sym2 << "push " << sym.stack_name << " " << sym.symbol_index;
        emit_vm_instruction(push_sym2.str());

        if (auto* class_type_ptr =
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

  const auto& call_site_subroutine_name =
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
  stringstream call_instr;
  call_instr << "call " << call_site_bind_name.str() << " " << call_site_args;
  emit_vm_instruction(call_instr.str());
}

void VmWriter::emit_vm_instruction(const std::string& instruction)
{
  if (left_justify_output)
  {
    // Left justify - no indentation
    lowered_vm << instruction << '\n';
  }
  else
  {
    // Check if this is a control flow instruction that should not be indented
    if (instruction.find("label ") == 0 || instruction.find("if-goto ") == 0 ||
        instruction.find("function ") == 0)
    {
      lowered_vm << instruction << '\n';
    }
    else
    {
      // All other instructions get 4 spaces of indentation (including goto)
      lowered_vm << "    " << instruction << '\n';
    }
  }
}

SymbolTable::VariableType_t VmWriter::get_expression_type(
    SubroutineDescr& subroutine_descr, const AstNode& expression_node)
{
  switch (expression_node.type)
  {
    case AstNodeType_t::N_INTEGER_CONSTANT:
      return SymbolTable::BasicType_t::T_INT;

    case AstNodeType_t::N_TRUE_KEYWORD:
    case AstNodeType_t::N_FALSE_KEYWORD:
      return SymbolTable::BasicType_t::T_BOOLEAN;

    case AstNodeType_t::N_VARIABLE_NAME:
      if (auto symbol =
              get_symbol_alloc_info(subroutine_descr, expression_node))
      {
        return symbol->variable_type;
      }
      break;

    case AstNodeType_t::N_OP_ADD:
    case AstNodeType_t::N_OP_SUBTRACT:
    case AstNodeType_t::N_OP_MULTIPLY:
    case AstNodeType_t::N_OP_DIVIDE:
      return SymbolTable::BasicType_t::T_INT;

    case AstNodeType_t::N_OP_LOGICAL_LT:
    case AstNodeType_t::N_OP_LOGICAL_GT:
    case AstNodeType_t::N_OP_LOGICAL_EQUALS:
      return SymbolTable::BasicType_t::T_BOOLEAN;

    case AstNodeType_t::N_OP_BITWISE_AND:
    case AstNodeType_t::N_OP_BITWISE_OR:
      if (expression_node.num_child_nodes() > 0)
      {
        return get_expression_type(subroutine_descr,
                                   expression_node.get_child_nodes()[0].get());
      }
      break;

    case AstNodeType_t::N_OP_PREFIX_BITWISE_NOT:
      if (expression_node.num_child_nodes() > 0)
      {
        return get_expression_type(subroutine_descr,
                                   expression_node.get_child_nodes()[0].get());
      }
      break;

    case AstNodeType_t::N_STRING_CONSTANT:
      return std::string("String");

    case AstNodeType_t::N_NULL_KEYWORD:
      return std::monostate {};

    default:
      break;
  }

  return std::monostate {};
}

bool VmWriter::are_types_compatible(const SymbolTable::VariableType_t& type1,
                                    const SymbolTable::VariableType_t& type2,
                                    AstNodeType_t operator_type)
{
  auto* basic_type1 = std::get_if<SymbolTable::BasicType_t>(&type1);
  auto* basic_type2 = std::get_if<SymbolTable::BasicType_t>(&type2);

  if (!basic_type1 || !basic_type2)
  {
    return true;
  }

  switch (operator_type)
  {
    case AstNodeType_t::N_OP_ADD:
    case AstNodeType_t::N_OP_SUBTRACT:
    case AstNodeType_t::N_OP_MULTIPLY:
    case AstNodeType_t::N_OP_DIVIDE:
      return (*basic_type1 == SymbolTable::BasicType_t::T_INT) &&
             (*basic_type2 == SymbolTable::BasicType_t::T_INT);

    case AstNodeType_t::N_OP_LOGICAL_LT:
    case AstNodeType_t::N_OP_LOGICAL_GT:
    case AstNodeType_t::N_OP_LOGICAL_EQUALS:
      return (*basic_type1 == SymbolTable::BasicType_t::T_INT) &&
             (*basic_type2 == SymbolTable::BasicType_t::T_INT);

    case AstNodeType_t::N_OP_BITWISE_AND:
    case AstNodeType_t::N_OP_BITWISE_OR:
      return *basic_type1 == *basic_type2;

    default:
      return true;
  }
}

void VmWriter::validate_binary_operator_types(SubroutineDescr& subroutine_descr,
                                              const AstNode& operator_node)
{
  if (operator_node.num_child_nodes() != 2)
  {
    return;
  }

  const auto& left_operand = operator_node.get_child_nodes()[0].get();
  const auto& right_operand = operator_node.get_child_nodes()[1].get();

  auto left_type = get_expression_type(subroutine_descr, left_operand);
  auto right_type = get_expression_type(subroutine_descr, right_operand);

  if (!are_types_compatible(left_type, right_type, operator_node.type))
  {
    auto* left_basic = std::get_if<SymbolTable::BasicType_t>(&left_type);
    auto* right_basic = std::get_if<SymbolTable::BasicType_t>(&right_type);

    if (left_basic || right_basic)
    {
      switch (operator_node.type)
      {
        case AstNodeType_t::N_OP_ADD:
        case AstNodeType_t::N_OP_SUBTRACT:
        case AstNodeType_t::N_OP_MULTIPLY:
        case AstNodeType_t::N_OP_DIVIDE:
          if ((left_basic &&
               *left_basic == SymbolTable::BasicType_t::T_BOOLEAN) ||
              (right_basic &&
               *right_basic == SymbolTable::BasicType_t::T_BOOLEAN))
          {
            emit_warning(operator_node,
                         "Arithmetic operation with boolean operand");
          }
          break;

        case AstNodeType_t::N_OP_LOGICAL_LT:
        case AstNodeType_t::N_OP_LOGICAL_GT:
        case AstNodeType_t::N_OP_LOGICAL_EQUALS:
          if ((left_basic &&
               *left_basic == SymbolTable::BasicType_t::T_BOOLEAN) ||
              (right_basic &&
               *right_basic == SymbolTable::BasicType_t::T_BOOLEAN))
          {
            emit_warning(operator_node,
                         "Comparison operation with boolean operand");
          }
          break;

        case AstNodeType_t::N_OP_BITWISE_AND:
        case AstNodeType_t::N_OP_BITWISE_OR:
          emit_warning(
              operator_node,
              "Mixed types in logical operation (operands must match)");
          break;

        default:
          break;
      }
    }
  }
}

void VmWriter::check_assignment_type_conversion(
    const SymbolTable::VariableType_t& lhs_type,
    const SymbolTable::VariableType_t& rhs_type, const AstNode& rhs_node)
{
  auto* lhs_basic = std::get_if<SymbolTable::BasicType_t>(&lhs_type);
  auto* rhs_basic = std::get_if<SymbolTable::BasicType_t>(&rhs_type);

  if (lhs_basic && rhs_basic)
  {
    if (*lhs_basic == SymbolTable::BasicType_t::T_BOOLEAN &&
        *rhs_basic == SymbolTable::BasicType_t::T_INT)
    {
      emit_warning(rhs_node, "Implicit conversion from int to boolean");
    }
    else if (*lhs_basic == SymbolTable::BasicType_t::T_INT &&
             *rhs_basic == SymbolTable::BasicType_t::T_BOOLEAN)
    {
      throw SemanticException("an int value is expected", rhs_node.line_number);
    }
  }
}

void VmWriter::validate_boolean_context(SubroutineDescr& subroutine_descr,
                                        const AstNode& expression_node,
                                        const std::string& context_name)
{
  auto condition_type = get_expression_type(subroutine_descr, expression_node);
  auto* basic_type = std::get_if<SymbolTable::BasicType_t>(&condition_type);

  if (!basic_type || *basic_type != SymbolTable::BasicType_t::T_BOOLEAN)
  {
    std::string message =
        "Non-boolean expression used in " + context_name + " condition";
    emit_warning(expression_node, message);
  }
}

void VmWriter::emit_warning(const AstNode& node, const std::string& message)
{
  std::cerr << "Warning: " << message;
  if (node.line_number > 0)
  {
    std::cerr << " (line " << node.line_number << ")";
  }
  std::cerr << std::endl;
}

bool VmWriter::has_return_statement(const AstNode& statement_block) const
{
  for (const auto& node : statement_block.get_child_nodes())
  {
    if (node.get().type == AstNodeType_t::N_RETURN_STATEMENT)
    {
      return true;
    }

    // Check nested blocks in if/while statements
    if (node.get().type == AstNodeType_t::N_IF_STATEMENT ||
        node.get().type == AstNodeType_t::N_WHILE_STATEMENT)
    {
      for (const auto& child : node.get().get_child_nodes())
      {
        if (child.get().type == AstNodeType_t::N_STATEMENT_BLOCK)
        {
          if (has_return_statement(child.get()))
          {
            return true;
          }
        }
      }
    }
  }
  return false;
}

void VmWriter::validate_return_statement(
    const SubroutineDescr& subroutine_descr, const AstNode& statement_block,
    const AstNode& subroutine_root)
{
  if (!has_return_statement(statement_block))
  {
    std::string message = "Function missing return statement";
    emit_warning(subroutine_root, message);
  }
}

}  // namespace jfcl
