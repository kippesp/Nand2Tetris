#include <cassert>
#include <string>

// clang-format off
#include "semantic_exception.h"
#include "symbol_table.h"
#include "vmwriter.h"
// clang-format on

ostream& operator<<(ostream& os, const Symbol& rhs)
{
  os << rhs.name << "\t";

  if (auto pBType = get_if<Symbol::BasicType_t>(&rhs.type))
  {
    switch (*pBType)
    {
      case Symbol::BasicType_t::T_INT:
        os << "int";
        break;
      case Symbol::BasicType_t::T_CHAR:
        os << "char";
        break;
      case Symbol::BasicType_t::T_BOOLEAN:
        os << "boolean";
        break;
      case Symbol::BasicType_t::T_VOID:
        os << "void";
        break;
    }
  }
  else if (auto pCType = get_if<Symbol::ClassType_t>(&rhs.type))
  {
    os << *pCType;
  }

  os << "\t";

  if (auto pCClass = get_if<Symbol::ClassStorageClass_t>(&rhs.storage_class))
  {
    switch (*pCClass)
    {
      case Symbol::ClassStorageClass_t::S_STATIC:
        os << "static";
        break;
      case Symbol::ClassStorageClass_t::S_FIELD:
        os << "field";
        break;
    }
  }
  else if (auto pSClass =
               get_if<Symbol::SubroutineStorageClass_t>(&rhs.storage_class))
  {
    switch (*pSClass)
    {
      case Symbol::SubroutineStorageClass_t::S_ARGUMENT:
        os << "argument";
        break;
      case Symbol::SubroutineStorageClass_t::S_LOCAL:
        os << "local";
        break;
    }
  }

  os << "\t";

  os << rhs.var_index;

  return os;
}

ostream& operator<<(ostream& os, const PartialSymbolTable& rhs)
{
  for (auto s : rhs.symbols) os << s << endl;
  ;

  return os;
}

ostream& operator<<(ostream& os, const SymbolTable& rhs)
{
  os << "CLASS LEVEL:" << endl;

  if (rhs.class_tbl)
  {
    os << *rhs.class_tbl;
  }
  else
  {
    os << " (none)\b";
  }

  os << "SUBROUTINE LEVEL:" << endl;

  if (rhs.subroutine_tbl)
  {
    os << *rhs.subroutine_tbl;
  }
  else
  {
    os << " (none)\b";
  }

  return os;
}

void Symbol::print() const
{
  cout << *this << endl;
}

void SymbolTable::print() const
{
  cout << *this << endl;
}

void SymbolTable::add_symbol(Symbol::ScopeLevel_t scope_level,
                             const std::string& varname,
                             Symbol::VariableType_t vartype,
                             Symbol::StorageClass_t storage_class)
{
  int varindex = 0;

  PartialSymbolTable* pST = nullptr;

  switch (scope_level)
  {
    case Symbol::ScopeLevel_t::CLASS:
      pST = &(*class_tbl);
      break;
    case Symbol::ScopeLevel_t::SUBROUTINE:
      pST = &(*subroutine_tbl);
      break;
  }

  if (const auto pCSC = get_if<Symbol::ClassStorageClass_t>(&storage_class))
  {
    if (scope_level != Symbol::ScopeLevel_t::CLASS)
      throw SemanticException("Unexpected non-class variable", varname);

    switch (*pCSC)
    {
      case Symbol::ClassStorageClass_t::S_STATIC:
        varindex = pST->next_index.static_var++;
        break;
      case Symbol::ClassStorageClass_t::S_FIELD:
        varindex = pST->next_index.field_var++;
        break;
    }
  }
  else if (const auto pSSC =
               get_if<Symbol::SubroutineStorageClass_s>(&storage_class))
  {
    if (scope_level != Symbol::ScopeLevel_t::SUBROUTINE)
      throw SemanticException("Unexpected non-subroutine variable", varname);

    switch (*pSSC)
    {
      case Symbol::SubroutineStorageClass_t::S_ARGUMENT:
        varindex = pST->next_index.argument_var++;
        break;
      case Symbol::SubroutineStorageClass_t::S_LOCAL:
        varindex = pST->next_index.local_var++;
        break;
    }
  }
  else
  {
    assert(0 && "fallthrough");
  }

  pST->symbols.push_back(Symbol(varname, vartype, storage_class, varindex));

  // Check if variable name already defined in table
  if (auto idx = pST->symbols.size() - 1;
      pST->symbol_names.find(varname) == pST->symbol_names.end())
  {
    pST->symbol_names[varname] = idx;
  }
  else
  {
    throw SemanticException("Duplicate variable at the same scoping", varname);
  }
}

const Symbol* SymbolTable::find_symbol(const string& varname) const
{
  if (auto TI = subroutine_tbl->symbol_names.find(varname);
      TI != subroutine_tbl->symbol_names.end())
  {
    auto idx = TI->second;
    const Symbol* symbol = &(subroutine_tbl->symbols[idx]);
    return symbol;
  }
  else if (TI = class_tbl->symbol_names.find(varname);
           TI != class_tbl->symbol_names.end())
  {
    auto idx = TI->second;
    const Symbol* symbol = &(class_tbl->symbols[idx]);

    if (pSubroutineDescr && symbol &&
        (pSubroutineDescr->type == TokenValue_t::J_FUNCTION) &&
        get_if<Symbol::ClassStorageClass_t>(&symbol->storage_class) &&
        (get<Symbol::ClassStorageClass_t>(symbol->storage_class) ==
         Symbol::ClassStorageClass_t::S_FIELD))
    {
      return nullptr;
    }

    return symbol;
  }
  else
  {
    return nullptr;
  }
}

void SymbolTable::reset(Symbol::ScopeLevel_t type)
{
  switch (type)
  {
    case (Symbol::ScopeLevel_t::SUBROUTINE):
      subroutine_tbl.reset();
      subroutine_tbl = std::make_unique<PartialSymbolTable>();
      pSubroutineDescr = nullptr;
      break;
    case (Symbol::ScopeLevel_t::CLASS):
      class_tbl.reset();
      class_tbl = std::make_unique<PartialSymbolTable>();
      break;
  }
}

Symbol::VariableType_t Symbol::variable_type_from_token(const JackToken& token)
{
  Symbol::VariableType_t variable_type;

  if (token.value_enum == TokenValue_t::J_INT)
    variable_type = Symbol::BasicType_t::T_INT;
  else if (token.value_enum == TokenValue_t::J_CHAR)
    variable_type = Symbol::BasicType_t::T_CHAR;
  else if (token.value_enum == TokenValue_t::J_BOOLEAN)
    variable_type = Symbol::BasicType_t::T_BOOLEAN;
  else if (token.value_enum == TokenValue_t::J_VOID)
    variable_type = Symbol::BasicType_t::T_VOID;
  else if (token.type == TokenType_t::J_IDENTIFIER)
    variable_type = Symbol::ClassType_t{token.value_str};

  return variable_type;
}
