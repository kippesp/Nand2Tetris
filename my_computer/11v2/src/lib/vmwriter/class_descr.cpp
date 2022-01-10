#include "class_descr.h"

using namespace std;
using namespace ast;

ClassDescr::ClassDescr(string name, AstNodeCRef root) : root(root), name(name)
{
}
