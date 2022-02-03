#include "class_descr.h"

using namespace std;
using namespace ast;

ClassDescr::ClassDescr(string name_, AstNodeCRef root_) : root(root_), name(name_)
{
}
