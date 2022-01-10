#include <cassert>
#include <string>

#include "semantic_exception.h"
#include "symbol_table.h"

using namespace std;
using namespace ast;

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

ClassSymbolTable::ClassSymbolTable() : SymbolTable(ScopeLevel_t::CLASS)
{
}

void ClassSymbolTable::add_symbol(std::string name, std::string type,
                                  std::string storage_class_in)
{
  if ((storage_class_in != "static") && (storage_class_in != "field"))
  {
    throw SemanticException("Class variable must be 'static' or 'field'", name);
  }

  StorageClass_t storage_class = (storage_class_in == "static")
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
    default:
      variable_index = -1;
      assert(0 && "fall through");
  }

  VariableType_t variable_type = variable_type_from_string(type);

  if (symbols.find(name) != symbols.end())
  {
    throw SemanticException("Class variable already defined", name);
  }

  symbols[name] = SymbolDescr_t(ScopeLevel_t::CLASS, variable_type,
                                storage_class, variable_index);
}

void SubroutineSymbolTable::add_symbol(std::string name, std::string type,
                                       std::string storage_class_in)
{
  if ((storage_class_in != "argument") && (storage_class_in != "local"))
  {
    throw SemanticException("Subroutine variable must be 'argument' or 'local'",
                            name);
  }

  StorageClass_t storage_class = (storage_class_in == "argument")
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
    default:
      variable_index = -1;
      assert(0 && "fall through");
  }

  VariableType_t variable_type = variable_type_from_string(type);

  if (symbols.find(name) != symbols.end())
  {
    throw SemanticException("Subroutine variable already defined", name);
  }

  symbols[name] = SymbolDescr_t(ScopeLevel_t::SUBROUTINE, variable_type,
                                storage_class, variable_index);
}
