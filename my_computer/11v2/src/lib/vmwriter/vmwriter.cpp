#include <functional>
#include <queue>

#include "class_descr.h"
#include "semantic_exception.h"
#include "vmwriter/vmwriter.h"

#include <signal.h>

// #include <functional>

// #include <iostream>

using namespace std;
using namespace ast;

namespace VmWriter {

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
T VmWriter::get_ast_node_value(AstNodeCRef node, AstNodeType_t node_type)
{
  AstNodeCRef ast_node = module_ast.find_child_node(node, node_type);

  if (ast_node.get() == EmptyNodeRef.get())
  {
    // raise(SIGTRAP);
    throw SemanticException("Expected node type not found in child nodes");
  }

  if (const auto s_ptr(get_if<T>(&ast_node.get().value)); s_ptr)
  {
    return *s_ptr;
  }

  throw SemanticException("Found child node's value is not of correct type");
}

template <typename T>
T VmWriter::get_ast_node_value(AstNodeCRef node)
{
  if (node.get() == EmptyNodeRef.get())
  {
    // raise(SIGTRAP);
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

void VmWriter::lower_class(AstNodeCRef root)
{
  auto class_name = get_ast_node_value<std::string>(root);
  ClassDescr& class_descr = program.add_class(class_name, root);

  // Add any class variables to the class symbol table
  if (AstNodeCRef class_variables_block =
          module_ast.find_child_node(root, AstNodeType_t::N_CLASS_VARIABLES);
      class_variables_block.get() != EmptyNodeRef.get())
  {
    for (auto& node : class_variables_block.get().get_child_nodes())
    {
      if (node.get().type != AstNodeType_t::N_VARIABLE_DECL)
      {
        throw SemanticException("Expected class variable decl node");
      }

      auto variable_name = get_ast_node_value<std::string>(node);
      auto variable_scope = get_ast_node_value<std::string>(
          node, AstNodeType_t::N_CLASS_VARIABLE_SCOPE);
      auto variable_type =
          get_ast_node_value<std::string>(node, AstNodeType_t::N_VARIABLE_TYPE);

      class_descr.add_symbol(variable_name, variable_scope, variable_type);
    }
  }

  auto child_nodes = root.get().get_child_nodes();
  for (auto& node : child_nodes)
  {
    switch (node.get().type)
    {
      case AstNodeType_t::N_FUNCTION_DECL:
      case AstNodeType_t::N_METHOD_DECL:
      case AstNodeType_t::N_CONSTRUCTOR_DECL:
        lower_subroutine(class_descr, node.get());
        break;
      default:
        throw SemanticException("Expected a subroutine declaration");
    }
  }
}

// Handle subroutine node types
//
// N_FUNCTION_DECL
//
// N_METHOD_DECL
//
// N_CONSTRUCTOR_DECL
//
void VmWriter::lower_subroutine(ClassDescr& class_descr, const AstNode& root)
{
  auto subroutine_name = get_ast_node_value<std::string>(root);
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

  // std::function<void(ast::AstNodeCRef)> build_dfs_nodes;

  // Helper function to add to subroutine's symbol table
  auto add_symbol = [&](ast::AstNodeType_t node_type) -> void {
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

        auto variable_name = get_ast_node_value<std::string>(node);
        auto variable_type = get_ast_node_value<std::string>(
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

  for (auto& node : StatementBlockNode.get().get_child_nodes())
  {
    switch (node.get().type)
    {
      case AstNodeType_t::N_RETURN_STATEMENT:
        lower_return_statement(subroutine_descr, node.get());
        break;
      case AstNodeType_t::N_LET_STATEMENT:
        throw SemanticException("statement not implemented");
        break;
      case AstNodeType_t::N_DO_STATEMENT:
        throw SemanticException("statement not implemented");
        break;
      case AstNodeType_t::N_WHILE_STATEMENT:
        throw SemanticException("statement not implemented");
        break;
      case AstNodeType_t::N_IF_STATEMENT:
        throw SemanticException("statement not implemented");
        break;
      default:
        throw SemanticException("fallthrough");
    }
  }
}

string VmWriter::lower_expression(SubroutineDescr& subroutine_descr,
                                  const AstNode& expression_parent_root)
{
  if (expression_parent_root.num_child_nodes() == 0)
  {
    throw SemanticException("statement node has no children");
  }

  if (expression_parent_root.num_child_nodes() > 1)
  {
    throw SemanticException("statement node has multiple children");
  }

  AstNodeCRef expression_root = expression_parent_root.get_child_nodes()[0];

  queue<AstNodeCRef> worklist;

  function<void(AstNodeCRef)> visit_fn;

  visit_fn = [&](AstNodeCRef new_node) {
    if (new_node.get().num_child_nodes() == 0)
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
    else if (node_type == AstNodeType_t::N_KEYWORD_CONSTANT)
    {
      // AST shouldn't have been designed to have this node.  It convey nothing
      // and its child has already been lowered.
    }
    // TODO: ???
    else if (false)
    {
      throw SemanticException("Should not be here");
    }
    // handle operators
    else
    {
      string operator_vm;

      switch (node_type)
      {
        case AstNodeType_t::N_OP_MULTIPLY:
          lowered_vm << "call Math.multiply 2" << endl;
          break;
        case AstNodeType_t::N_OP_DIVIDE:
          lowered_vm << "call Math.divide 2" << endl;
          break;
        case AstNodeType_t::N_OP_ADD:
          lowered_vm << "add" << endl;
          break;
        case AstNodeType_t::N_OP_SUBTRACT:
          lowered_vm << "sub" << endl;
          break;
        case AstNodeType_t::N_OP_LOGICAL_EQUALS:
          lowered_vm << "eq" << endl;
          break;
        case AstNodeType_t::N_OP_LOGICAL_GT:
          lowered_vm << "gt" << endl;
          break;
        case AstNodeType_t::N_OP_LOGICAL_LT:
          lowered_vm << "lt" << endl;
          break;
        case AstNodeType_t::N_OP_LOGICAL_AND:
          lowered_vm << "and" << endl;
          break;
        case AstNodeType_t::N_OP_LOGICAL_OR:
          lowered_vm << "or" << endl;
          break;
        case AstNodeType_t::N_OP_PREFIX_NEG:
          lowered_vm << "neg" << endl;
          break;
        case AstNodeType_t::N_OP_PREFIX_LOGICAL_NOT:
          lowered_vm << "not" << endl;
          break;
        case AstNodeType_t::N_TRUE_KEYWORD:
          lowered_vm << "push constant 0" << endl;
          lowered_vm << "not" << endl;
          break;
        case AstNodeType_t::N_FALSE_KEYWORD:
          lowered_vm << "push constant 0" << endl;
          break;
        default:
        {
          // throw SemanticException("expression fallthrough");
          // raise(SIGTRAP);
          stringstream ss;
          ss << "expression fallthrough:\n";
          ss << node.get();
          throw SemanticException(ss.str());
        }
      }
    }
  }

  return "";
}

void VmWriter::lower_return_statement(SubroutineDescr& subroutine_descr,
                                      const AstNode& root)
{
  string lowered_expression_vm = (root.num_child_nodes() > 0)
                                     ? lower_expression(subroutine_descr, root)
                                     : "";

  if (subroutine_descr.get_root().get() == AstNodeType_t::N_FUNCTION_DECL)
  {
    if (auto return_type_ptr = get_if<SymbolTable::BasicType_t>(
            &subroutine_descr.get_return_type());
        return_type_ptr)
    {
      if (*return_type_ptr != SymbolTable::BasicType_t::T_VOID)
      {
        // TODO: ?? Check return is of void type
      }

      lowered_vm << "push constant 0 " << endl;
      lowered_vm << "return" << endl;
    }
  }
  else
  {
    lowered_vm << lowered_expression_vm;

    if (subroutine_descr.get_root().get() == AstNodeType_t::N_METHOD_DECL)
    {
      lowered_vm << "push constant 0" << endl;
    }

    lowered_vm << "return" << endl;
  }
}

}  // namespace VmWriter
