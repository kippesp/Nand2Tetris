#pragma once

#include "parser/ast.h"
#include "semantic_exception.h"
#include "symbol_table.h"

#include <optional>

namespace jfcl {

class SubroutineDescr;
using SubroutineDescrRef = std::reference_wrapper<SubroutineDescr>;

class ClassDescr;
using ClassDescrRef = std::reference_wrapper<ClassDescr>;

class SubroutineDescr {
  ClassDescrRef class_descr;
  const AstNodeCRef root;
  const std::string name;
  SubroutineSymbolTable symbol_table;
  SymbolTable::VariableType_t return_type;

  // counter for if-else and while control statements
  int structured_control_id {0};

public:
  SubroutineDescr() = delete;
  SubroutineDescr(const SubroutineDescr&) = delete;
  SubroutineDescr& operator=(const SubroutineDescr&) = delete;

  // move constructor
  SubroutineDescr(SubroutineDescr&&) = default;

  SubroutineDescr(ClassDescrRef, std::string, SymbolTable::VariableType_t,
                  AstNodeCRef);

  const AstNode& get_root() const { return root.get(); }
  const std::string& get_name() const { return name; }
  const std::string& get_class_name() const;
  const SymbolTable::VariableType_t& get_return_type() const
  {
    return return_type;
  }

  void add_symbol(const std::string& arg_name, const std::string& scope,
                  const std::string& class_name)
  {
    symbol_table.add_symbol(arg_name, scope, class_name);
  }

  void add_symbol(const std::string& arg_name, const std::string& scope,
                  const std::string& class_name,
                  std::function<void(const std::string&)> warn_func)
  {
    symbol_table.add_symbol(arg_name, scope, class_name, warn_func);
  }

  int num_locals() const { return symbol_table.num_locals(); }
  int num_fields() const;

  std::optional<Symbol> find_symbol(std::string);

  int get_next_structured_control_id() { return structured_control_id++; }
};

}  // namespace jfcl
