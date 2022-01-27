#include "subroutine_descr.h"
#include "class_descr.h"

using namespace std;
using namespace ast;

SubroutineDescr::SubroutineDescr(ClassDescrRef class_descr, string name,
                                 SymbolTable::VariableType_t return_type,
                                 AstNodeCRef root)
    : class_descr(class_descr), root(root), name(name), return_type(return_type)
{
}

int SubroutineDescr::num_fields() const
{
  return class_descr.get().get_symbol_table().num_fields();
}

optional<Symbol> SubroutineDescr::find_symbol(std::string symbol_name)
{
  // Search subroutine symbols (this hides any def in the class)
  if (auto symbol = symbol_table.symbols.find(symbol_name);
      symbol != symbol_table.symbols.end())
  {
    return Symbol(symbol->second);
  }

  // Search class symbols
  if (auto symbol = class_descr.get().symbol_table.symbols.find(symbol_name);
      symbol != class_descr.get().symbol_table.symbols.end())
  {
    return Symbol(symbol->second);
  }

  return std::nullopt;
}
