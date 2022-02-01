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
const T& VmWriter::get_ast_node_value(AstNodeCRef node, AstNodeType_t node_type)
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
const T& VmWriter::get_ast_node_value(AstNodeCRef node)
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
  auto& subroutine_name = get_ast_node_value<std::string>(root);
  const AstNode& DescrNode =
      module_ast.find_child_node(root, AstNodeType_t::N_SUBROUTINE_DESCR).get();
  auto return_type = SymbolTable::variable_type_from_string(
      get_ast_node_value<string>(DescrNode, AstNodeType_t::N_RETURN_TYPE));

  SubroutineDescr& subroutine_descr =
      class_descr.add_subroutine(subroutine_name, return_type, root).get();

  // raise(SIGTRAP);

  // For class methods, add the implicit argument "this" representing the class
  if (root.type == AstNodeType_t::N_METHOD_DECL)
  {
    subroutine_descr.add_symbol("this", "argument", class_descr.get_name());
  }

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

  for (auto& node : StatementBlockNode.get().get_child_nodes())
  {
    switch (node.get().type)
    {
      case AstNodeType_t::N_RETURN_STATEMENT:
        lower_return_statement(subroutine_descr, node);
        break;
      case AstNodeType_t::N_LET_STATEMENT:
        throw SemanticException("statement not implemented");
        break;
      case AstNodeType_t::N_DO_STATEMENT:
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
      if (subroutine_descr.get_root().get().type ==
          AstNodeType_t::N_FUNCTION_DECL)
      {
        throw SemanticException("'this' keyword not permitted in functions");
      }

      lowered_vm << "push pointer 0" << endl;
    }
    else if (node_type == AstNodeType_t::N_VARIABLE_NAME)
    {
      auto& bind_var_name = get_ast_node_value<string>(node);

      if (auto bind_var = subroutine_descr.find_symbol(bind_var_name);
          bind_var.has_value())
      {
        string stack_name;

        switch (bind_var->scope_level)
        {
          case SymbolTable::ScopeLevel_t::CLASS:
            switch (bind_var->storage_class)
            {
              case SymbolTable::StorageClass_t::S_STATIC:
                stack_name = "static";
                break;
              case SymbolTable::StorageClass_t::S_FIELD:
                stack_name = "this";
                break;
              default:
                assert(0 && "invalid storage class for CLASS");
                throw SemanticException("invalid storage class for CLASS");
            }

            break;
          case SymbolTable::ScopeLevel_t::SUBROUTINE:
            switch (bind_var->storage_class)
            {
              case SymbolTable::StorageClass_t::S_ARGUMENT:
                stack_name = "argument";
                break;
              case SymbolTable::StorageClass_t::S_LOCAL:
                stack_name = "local";
                break;
              default:
                assert(0 && "invalid storage class for SUBROUTINE");
                throw SemanticException("invalid storage class for SUBROUTINE");
            }

            break;
        }

        lowered_vm << "push " << stack_name << " " << bind_var->index << endl;
      }
      else
      {
        throw SemanticException("Symbol not found:", bind_var_name);
      }
    }
    else if (node_type == AstNodeType_t::N_SUBROUTINE_CALL)
    {
      lower_subroutine_call(subroutine_descr, node);
    }
    else if (node_type == AstNodeType_t::N_SUBSCRIPTED_VARIABLE_NAME)
    {
      assert(0 && "array lowering not implemented");
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
  assert((root.num_child_nodes() == 0) || (root.num_child_nodes() == 1));

  string lowered_expression_vm =
      (root.num_child_nodes() > 0)
          ? lower_expression(subroutine_descr, root.get_child_nodes()[0].get())
          : "";

  auto subroutine_type = subroutine_descr.get_root().get().type;
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
      if (lowered_expression_vm != "")
      {
        throw SemanticException(
            "Function/method declared void but returning non-void");
      }

      lowered_vm << "push constant 0" << endl;
      lowered_vm << "return" << endl;
      return;
    }

    assert(subroutine_type == AstNodeType_t::N_CONSTRUCTOR_DECL);

    throw SemanticException("Constructor declared as void");
  }

  lowered_vm << lowered_expression_vm << "return" << endl;
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

  ///////
  // Lower call-site binding

  // Handle local calls into the current object's method.
  //    // Example:
  //    // The current class has a method, add(int)
  //    do let my_local_var = add(1);
  if (call_site.type == AstNodeType_t::N_LOCAL_CALL_SITE)
  {
    // If the symbol table has the 'this' variable, we're in a class method
    if (auto this_symbol = subroutine_descr.find_symbol("this");
        this_symbol.has_value())
    {
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
        assert(symbol_type_ptr && "Expected ClassType_t");
      }
    }
    else
    {
      // TODO: not here: call_site_bind_name << sub_name << ".";
      assert(0 && "Here 1");
    }
  }
  // Handle global calls into another object's method or class's constructor.
  //    // has a method, set(int)
  //    // has a constructor, new()
  //    var MyClass my_local_var;
  //
  //    // a global call site constructor call
  //    let my_local_var = MyClass.new();
  //
  //    // a global call site method call
  //    my_local_var.add(1);
  else if (call_site.type == AstNodeType_t::N_GLOBAL_CALL_SITE)
  {
    auto& global_bind_name = get_ast_node_value<string>(
        call_site, AstNodeType_t::N_GLOBAL_BIND_NAME);

    // Is this a locally defined object's METHOD where the global-bind-name is
    // found in the symbol table.
    if (auto bind_var = subroutine_descr.find_symbol(global_bind_name);
        bind_var.has_value())
    {
      assert(0 && "Here 2");
    }
    // No, the global-bind-name name must be a class name into another module
    // (possibly this module which is handled the same)
    else
    {
      // TODO: I think at this point we could be calling a function, method, or
      // constructor.  Need to think through why this works.

      // auto subroutine_type = subroutine_descr.get_root().get().type;
      // assert(subroutine_type == AstNodeType_t::N_CONSTRUCTOR_DECL);

      // provide 'this' pointer to internal method call
      // lowered_vm << "push pointer 0" << endl;
      // call_site_args++;

      call_site_bind_name << global_bind_name << ".";
    }
  }
  else
  {
    cout << call_site << endl;
    throw SemanticException("Unexpected fall through");
  }

  auto& call_site_subroutine_name =
      get_ast_node_value<string>(call_site, AstNodeType_t::N_SUBROUTINE_NAME);
  call_site_bind_name << call_site_subroutine_name;

  ///////
  //

  // TODO: Lower all of the
  // assert(root.num_child_nodes() == 1);

  const auto& call_args_node =
      module_ast.find_child_node(root, AstNodeType_t::N_CALL_ARGUMENTS).get();
  assert(call_args_node != EmptyNodeRef.get());

  for (auto& node : call_args_node.get_child_nodes())
  {
    lower_expression(subroutine_descr, node.get());
    call_site_args++;
  }

  lowered_vm << "call " << call_site_bind_name.str() << " " << call_site_args
             << endl;
}

}  // namespace VmWriter
