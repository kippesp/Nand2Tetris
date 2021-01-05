#pragma once

#include "parser/parse_tree.h"

class VmWriter {
public:
  VmWriter() = delete;
  VmWriter(const VmWriter&) = delete;
  VmWriter& operator=(const VmWriter&) = delete;

  virtual ~VmWriter() = default;

  // friend std::ostream& operator<<(std::ostream&, const VmWriter&);

  // Return visited node of parse tree using DFS
  const ParseTreeNode* visit();

  void init()
  {
    unvisited_nodes.clear();
    unvisited_nodes.push_back(parsetree_root_node);
  }

  VmWriter(const std::shared_ptr<ParseTreeNonTerminal> pt)
      : parsetree_root_node(&(*pt))
  {
    init();
  }

private:
  const ParseTreeNode* parsetree_root_node;
  std::vector<const ParseTreeNode*> unvisited_nodes;
};
