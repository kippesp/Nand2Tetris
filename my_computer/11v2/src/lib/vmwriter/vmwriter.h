#pragma once

#include "parser/ast.h"
#include "program.h"

#include <sstream>
#include <string>

namespace jfcl {

class VmWriter {
public:
  VmWriter() = delete;
  VmWriter(const VmWriter&) = delete;
  VmWriter& operator=(const VmWriter&) = delete;

  VmWriter(const AstTree& ast_tree, bool left_justify = false)
      : module_ast(ast_tree),
        module_root(ast_tree.get_root()),
        EmptyNodeRef(module_ast.get_empty_node_ref().get()),
        left_justify_output(left_justify)
  {
  }

  void lower_module();

  std::string get_lowered_vm() const { return lowered_vm.str(); }

private:
  using SymbolLoweringLocations_t = struct SymbolLoweringLocations {
    SymbolTable::ScopeLevel_t scope_level;
    SymbolTable::VariableType_t variable_type {};
    std::string stack_name {};
    int symbol_index {};
  };

  const AstTree& module_ast;
  AstNodeCRef module_root;

  AstNodeCRef EmptyNodeRef;

  void lower_class(AstNodeCRef);
  void lower_subroutine(ClassDescr&, const AstNode&);
  void lower_statement_block(SubroutineDescr&, const AstNode&);
  std::string lower_expression(SubroutineDescr&, const AstNode&);
  void lower_return_statement(SubroutineDescr&, const AstNode&);
  void lower_let_statement(SubroutineDescr&, const AstNode&);
  void lower_while_statement(SubroutineDescr&, const AstNode&);
  void lower_if_statement(SubroutineDescr&, const AstNode&);
  void lower_subroutine_call(SubroutineDescr&, const AstNode&);
  void lower_var(SubroutineDescr&, const AstNode&);

  // Helper to find symbol in symbol table and construct the approprate
  // VM stack name and stack index
  std::optional<SymbolLoweringLocations_t> get_symbol_alloc_info(
      SubroutineDescr&, const AstNode&);

  static std::optional<SymbolLoweringLocations_t> get_symbol_alloc_info(
      SubroutineDescr&, const std::string&);

  template <typename T>
  const T& get_ast_node_value(AstNodeCRef, AstNodeType_t);

  template <typename T>
  const T& get_ast_node_value(AstNodeCRef node);

  Program program;

  std::stringstream lowered_vm;

  bool left_justify_output;

  // Global label counter for unique label generation across all subroutines
  int global_label_counter {0};

  // Helper method to add indentation based on VM instruction type
  void emit_vm_instruction(const std::string& instruction);

  // Helper method to get next unique label ID
  int get_next_label_id() { return global_label_counter++; }

  // Type analysis methods for warning system
  SymbolTable::VariableType_t get_expression_type(
      SubroutineDescr& subroutine_descr, const AstNode& expression_node);

  bool are_types_compatible(const SymbolTable::VariableType_t& type1,
                            const SymbolTable::VariableType_t& type2,
                            AstNodeType_t operator_type);

  void validate_binary_operator_types(SubroutineDescr& subroutine_descr,
                                      const AstNode& operator_node);

  void check_assignment_type_conversion(
      const SymbolTable::VariableType_t& lhs_type,
      const SymbolTable::VariableType_t& rhs_type, const AstNode& rhs_node);

  void validate_boolean_context(SubroutineDescr& subroutine_descr,
                                const AstNode& expression_node,
                                const std::string& context_name);

  void emit_warning(const AstNode& node, const std::string& message);
};

}  // namespace jfcl
