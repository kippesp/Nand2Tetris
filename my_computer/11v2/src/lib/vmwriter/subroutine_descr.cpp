#include "subroutine_descr.h"

#include "class_descr.h"

using namespace std;

namespace jfcl {

SubroutineDescr::SubroutineDescr(ClassDescrRef class_descr_, string name_,
                                 SymbolTable::VariableType_t return_type_,
                                 AstNodeCRef root_)
    : class_descr(class_descr_),
      root(root_),
      name(name_),
      return_type(return_type_)
{
}

int SubroutineDescr::num_fields() const
{
  return class_descr.get().get_symbol_table().num_fields();
}

const std::string& SubroutineDescr::get_class_name() const
{
  return class_descr.get().get_name();
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

}  // namespace jfcl
