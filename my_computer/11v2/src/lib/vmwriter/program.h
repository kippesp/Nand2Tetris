#pragma once

#include <string>
#include <utility>
#include <vector>

#include "class_descr.h"
#include "parser/ast.h"

// Program stores the class descriptors
class Program {
public:
  Program(const Program&) = delete;
  Program& operator=(const Program&) = delete;

  Program() {}

  ClassDescr& add_class(std::string class_name, ast::AstNodeCRef class_node)
  {
    classes.emplace_back(ClassDescr(class_name, class_node));

    ClassDescr& rvalue = classes.back();
    return rvalue;
  }

private:
  // program name
  std::string name{};

  std::vector<ClassDescr> classes;
};
