#pragma once

#include <iostream>
#include <map>
#include <string>
#include <variant>
#include <vector>

#include "tokenizer/jack_tokenizer.h"

using namespace std;

struct SubroutineDescr;

// variable definition
class Symbol {
public:
  // Simplistic mechanism to verify class/subroutine variables are separate
  typedef enum class ScopeLevel_s {
    CLASS,
    SUBROUTINE,
  } ScopeLevel_t;

  // variable types
  typedef enum class BasicType_s {
    T_INT,
    T_CHAR,
    T_BOOLEAN,
    T_VOID,
  } BasicType_t;
  using ClassType_t = std::string;
  using VariableType_t = std::variant<std::monostate, BasicType_t, ClassType_t>;

  // variable storage-class types
  typedef enum class ClassStorageClass_s {
    S_STATIC,
    S_FIELD,
  } ClassStorageClass_t;
  typedef enum class SubroutineStorageClass_s {
    S_ARGUMENT,
    S_LOCAL,
  } SubroutineStorageClass_t;
  using StorageClass_t = std::variant<std::monostate, ClassStorageClass_t,
                                      SubroutineStorageClass_t>;

  Symbol() = delete;
  Symbol& operator=(const Symbol&) = delete;

  virtual ~Symbol() = default;

  Symbol(const std::string& n, VariableType_t t, StorageClass_t c, int i)
      : name(n), type(t), storage_class(c), var_index(i)
  {
  }

  friend std::ostream& operator<<(std::ostream&, const Symbol&);
  const std::string name;
  const VariableType_t type;
  const StorageClass_t storage_class;
  const int var_index;

  static Symbol::VariableType_t variable_type_from_token(
      const JackToken& token);

  void print() const;
};

// class or subroutine variable definitions
class PartialSymbolTable {
public:
  // PartialSymbolTable() = delete;
  PartialSymbolTable(const PartialSymbolTable&) = delete;
  PartialSymbolTable& operator=(const PartialSymbolTable&) = delete;

  virtual ~PartialSymbolTable() = default;

  PartialSymbolTable() : next_index({0, 0, 0, 0}) {}

  friend std::ostream& operator<<(std::ostream&, const PartialSymbolTable&);

  using symbol_index_t = std::vector<Symbol>::size_type;

  std::vector<Symbol> symbols;
  std::map<std::string, symbol_index_t> symbol_names;

  struct {
    int static_var;    // class-level
    int field_var;     // class-level
    int argument_var;  // subroutine-level
    int local_var;     // subroutine-level
  } next_index;
};

class SymbolTable {
public:
  SymbolTable(const SymbolTable&) = delete;
  SymbolTable& operator=(const SymbolTable&) = delete;

  virtual ~SymbolTable() = default;

  SymbolTable()
  {
#if 0
    class_tbl = std::make_unique<PartialSymbolTable>();
    subroutine_tbl = std::make_unique<PartialSymbolTable>();
#endif
  }

  friend std::ostream& operator<<(std::ostream&, const SymbolTable&);

  void add_symbol(Symbol::ScopeLevel_t, const std::string&,
                  Symbol::VariableType_t, Symbol::StorageClass_t);
  const Symbol* find_symbol(const std::string&) const;
  void reset(Symbol::ScopeLevel_t);
  void print() const;
  int num_locals() const { return subroutine_tbl->next_index.local_var; }
  SubroutineDescr* pSubroutineDescr{nullptr};

private:
  std::unique_ptr<PartialSymbolTable> class_tbl{nullptr};
  std::unique_ptr<PartialSymbolTable> subroutine_tbl{nullptr};
};
