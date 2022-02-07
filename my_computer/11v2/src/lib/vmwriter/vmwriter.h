#pragma once

#include "parser/ast.h"
#include "program.h"

namespace VmWriter {

class VmWriter {
public:
  VmWriter() = delete;
  VmWriter(const VmWriter&) = delete;
  VmWriter& operator=(const VmWriter&) = delete;

  VmWriter(const ast::AstTree& ast_tree)
      : module_ast(ast_tree),
        module_root(ast_tree.get_root()),
        EmptyNodeRef(module_ast.get_empty_node_ref().get())
  {
  }

  void lower_module();

  void dump_ast() const { (module_ast.get_root().get()).dump(); }

  std::string get_lowered_vm() const { return lowered_vm.str(); }

private:
  using SymbolLoweringLocations_t = struct SymbolLoweringLocations {
    SymbolTable::ScopeLevel_t scope_level;
    SymbolTable::VariableType_t variable_type {};
    std::string stack_name {};
    int symbol_index {};
  };

  const ast::AstTree& module_ast;
  ast::AstNodeCRef module_root;

  ast::AstNodeCRef EmptyNodeRef;

  void lower_class(ast::AstNodeCRef);
  void lower_subroutine(ClassDescr&, const ast::AstNode&);
  void lower_statement_block(SubroutineDescr&, const ast::AstNode&);
  std::string lower_expression(SubroutineDescr&, const ast::AstNode&);
  void lower_return_statement(SubroutineDescr&, const ast::AstNode&);
  void lower_let_statement(SubroutineDescr&, const ast::AstNode&);
  void lower_while_statement(SubroutineDescr&, const ast::AstNode&);
  void lower_if_statement(SubroutineDescr&, const ast::AstNode&);
  void lower_subroutine_call(SubroutineDescr&, const ast::AstNode&);

  // Helper to find symbol in symbol table and construct the approprate
  // VM stack name and stack index
  std::optional<SymbolLoweringLocations_t> get_symbol_lowering_locations(
      SubroutineDescr&, const ast::AstNode&);

  std::optional<SymbolLoweringLocations_t> get_symbol_lowering_locations(
      SubroutineDescr&, const std::string&);

  template <typename T>
  const T& get_ast_node_value(ast::AstNodeCRef, ast::AstNodeType_t);

  template <typename T>
  const T& get_ast_node_value(ast::AstNodeCRef node);

  Program program;

  std::stringstream lowered_vm;
};

}  // namespace VmWriter
