#include "vmwriter/vmwriter.h"

#include <algorithm>
#include <cassert>

#include "parser/parse_tree.h"


// bool: 0xFFFF


const ParseTreeNode* VmWriter::visit()
{
  if (unvisited_nodes.size() == 0) return nullptr;

  auto node = unvisited_nodes.back();
  unvisited_nodes.pop_back();

  if (auto term_node = dynamic_cast<const ParseTreeNonTerminal*>(node))
  {
    std::vector<std::shared_ptr<ParseTreeNode>> ncopy;
    for (auto child_node : term_node->get_child_nodes())
      ncopy.push_back(child_node);
    reverse(ncopy.begin(), ncopy.end());

    for (auto child_node : ncopy) unvisited_nodes.push_back(&(*child_node));
  }

  return node;
}
