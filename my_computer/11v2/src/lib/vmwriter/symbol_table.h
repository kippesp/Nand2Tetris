#pragma once

#include "parser/ast.h"

#include <map>

namespace jfcl {

class SymbolTable {
public:
  typedef enum class ScopeLevel_s {
    CLASS,
    SUBROUTINE,
  } ScopeLevel_t;

  typedef enum class StorageClass_s {
    // for classes
    S_STATIC,
    S_FIELD,
    // for subroutines
    S_ARGUMENT,
    S_LOCAL,
  } StorageClass_t;

  typedef enum class BasicType_s {
    T_INT,
    T_CHAR,
    T_BOOLEAN,
    T_VOID,
  } BasicType_t;

  using ClassType_t = std::string;
  using VariableType_t = std::variant<std::monostate, BasicType_t, ClassType_t>;
  using SymbolDescr_t =
      std::tuple<ScopeLevel_t, VariableType_t, StorageClass_t, int>;

  SymbolTable() {}

  static VariableType_t variable_type_from_string(std::string);
};

class Symbol {
public:
  Symbol(const SymbolTable::SymbolDescr_t& s)
      : scope_level(get<0>(s)),
        variable_type(get<1>(s)),
        storage_class(get<2>(s)),
        index(get<3>(s))
  {
  }
  const SymbolTable::ScopeLevel_t scope_level;
  const SymbolTable::VariableType_t variable_type;
  const SymbolTable::StorageClass_t storage_class;
  const int index;
};

class ClassSymbolTable : public SymbolTable {
public:
  ClassSymbolTable();

  void add_symbol(const std::string& symbol_name, const std::string& scope,
                  const std::string& symbol_type);

  int num_fields() const { return next_storage_class_index.field_var; }

  std::map<std::string, SymbolDescr_t> symbols;

private:
  struct StorageClassIndicies_s {
    int static_var;
    int field_var;
  } next_storage_class_index {0, 0};
};

class SubroutineSymbolTable : public SymbolTable {
public:
  SubroutineSymbolTable() : SymbolTable() {}

  void add_symbol(const std::string& symbol_name, const std::string& scope,
                  const std::string& symbol_type);

  int num_locals() const { return next_storage_class_index.local_var; }

  std::map<std::string, SymbolDescr_t> symbols;

private:
  struct StorageClassIndicies_s {
    int argument_var;
    int local_var;
  } next_storage_class_index {0, 0};
};

}  // namespace jfcl
