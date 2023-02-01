#include "semantic_exception.h"
#include "symbol_table.h"

using namespace std;

namespace jfcl {

SymbolTable::VariableType_t SymbolTable::variable_type_from_string(
    std::string in_type)
{
  VariableType_t variable_type;

  if (in_type == "int")
  {
    variable_type = BasicType_t::T_INT;
  }
  else if (in_type == "char")
  {
    variable_type = BasicType_t::T_CHAR;
  }
  else if (in_type == "boolean")
  {
    variable_type = BasicType_t::T_BOOLEAN;
  }
  else if (in_type == "void")
  {
    variable_type = BasicType_t::T_VOID;
  }
  else
  {
    variable_type = in_type;  // a.k.a. ClassType_t
  }

  return variable_type;
}

ClassSymbolTable::ClassSymbolTable() : SymbolTable() {}

void ClassSymbolTable::add_symbol(const std::string& symbol_name,
                                  const std::string& storage_class_str,
                                  const std::string& symbol_type_str)
{
  StorageClass_t storage_class = (storage_class_str == "static")
                                     ? StorageClass_t::S_STATIC
                                     : StorageClass_t::S_FIELD;

  int variable_index;

  switch (storage_class)
  {
    case StorageClass_t::S_STATIC:
      variable_index = next_storage_class_index.static_var++;
      break;
    case StorageClass_t::S_FIELD:
      variable_index = next_storage_class_index.field_var++;
      break;
    case StorageClass_t::S_ARGUMENT:
      throw SemanticException(
          "Storage class 'argument' not permitted in class");
    case StorageClass_t::S_LOCAL:
      throw SemanticException("Storage class 'var' not permitted in class");
  }

  VariableType_t symbol_type = variable_type_from_string(symbol_type_str);

  if (symbols.find(symbol_name) != symbols.end())
  {
    throw SemanticException("Class variable already defined", symbol_name);
  }

  symbols[symbol_name] = SymbolDescr_t(ScopeLevel_t::CLASS, symbol_type,
                                       storage_class, variable_index);
}

void SubroutineSymbolTable::add_symbol(const std::string& symbol_name,
                                       const std::string& storage_class_str,
                                       const std::string& symbol_type_str)
{
  if ((storage_class_str != "argument") && (storage_class_str != "local"))
  {
    throw SemanticException(
        "Subroutine variable storage class must be 'argument' or 'local'",
        symbol_name);
  }

  StorageClass_t storage_class = (storage_class_str == "argument")
                                     ? StorageClass_t::S_ARGUMENT
                                     : StorageClass_t::S_LOCAL;

  int variable_index;

  switch (storage_class)
  {
    case StorageClass_t::S_ARGUMENT:
      variable_index = next_storage_class_index.argument_var++;
      break;
    case StorageClass_t::S_LOCAL:
      variable_index = next_storage_class_index.local_var++;
      break;
    case StorageClass_t::S_STATIC:
      throw SemanticException(
          "Storage class 'static' not permitted in subroutines");
    case StorageClass_t::S_FIELD:
      throw SemanticException(
          "Storage class 'field' not permitted in subroutines");
  }

  VariableType_t symbol_type = variable_type_from_string(symbol_type_str);

  if (symbols.find(symbol_name) != symbols.end())
  {
    throw SemanticException("Subroutine variable already defined", symbol_name);
  }

  symbols[symbol_name] = SymbolDescr_t(ScopeLevel_t::SUBROUTINE, symbol_type,
                                       storage_class, variable_index);
}

}  // namespace jfcl
