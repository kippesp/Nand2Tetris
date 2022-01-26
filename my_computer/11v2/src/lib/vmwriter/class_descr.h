#pragma once

#include "semantic_exception.h"
#include "subroutine_descr.h"
#include "symbol_table.h"

class ClassDescr;
using ClassDescrRef = std::reference_wrapper<ClassDescr>;

// Provides the class definition such as class name, class symbol table, and
// subroutine definition references.
class ClassDescr {
  const ast::AstNodeCRef root;
  std::vector<SubroutineDescr> subroutines;
  const std::string name;

public:
  ClassDescr() = delete;
  ClassDescr(const ClassDescr&) = delete;
  ClassDescr& operator=(const ClassDescr&) = delete;

  // move constructor
  ClassDescr(ClassDescr&&) = default;

  virtual ~ClassDescr() = default;

  ClassDescr(std::string, ast::AstNodeCRef);

  const std::string& get_name() const { return name; }

  SubroutineDescrRef add_subroutine(std::string subroutine_name,
                                    SymbolTable::VariableType_t return_type,
                                    ast::AstNodeCRef subroutine_node)
  {
    subroutines.emplace_back(
        SubroutineDescr(*this, subroutine_name, return_type, subroutine_node));

    SubroutineDescrRef rvalue = subroutines.back();
    return rvalue;
  }

  void add_symbol(const std::string& symbol_name, const std::string& scope,
                  const std::string& symbol_type)
  {
    symbol_table.add_symbol(symbol_name, scope, symbol_type);
  }

  const ClassSymbolTable& get_symbol_table() const { return symbol_table; }

  ClassSymbolTable symbol_table;
};
