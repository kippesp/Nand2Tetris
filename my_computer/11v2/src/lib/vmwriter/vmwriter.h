#pragma once

#include "parser/ast.h"
#include "program.h"

namespace VmWriter {

class VmWriter {
public:
  VmWriter() = delete;
  VmWriter(const VmWriter&) = delete;
  VmWriter& operator=(const VmWriter&) = delete;

  virtual ~VmWriter() = default;

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
  const ast::AstTree& module_ast;
  ast::AstNodeCRef module_root;

  ast::AstNodeCRef EmptyNodeRef;

  void lower_class(ast::AstNodeCRef);
  void lower_subroutine(ClassDescr&, const ast::AstNode&);
  void lower_statement_block(ast::AstNodeCRef);
  std::string lower_expression(SubroutineDescr&, const ast::AstNode&);
  void lower_return_statement(SubroutineDescr&, const ast::AstNode&);
  void lower_subroutine_call(SubroutineDescr&, const ast::AstNode&);

  template <typename T>
  const T& get_ast_node_value(ast::AstNodeCRef, ast::AstNodeType_t);

  template <typename T>
  const T& get_ast_node_value(ast::AstNodeCRef node);

#if 0
  template <typename T>
  T get_ast_node_value(const ast::AstNode& node)
  {
    return get_ast_node_value<T>(ast::AstNodeCRef(node));
  }
#endif

  Program program;

  std::stringstream lowered_vm;
};

}  // namespace VmWriter
