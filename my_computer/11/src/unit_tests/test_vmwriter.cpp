#include <signal.h>
#include <string.h>

#include "catch.hpp"
#include "jack_sources.h"
#include "mock_reader.h"
#include "parser/parse_tree.h"
#include "vmwriter/vmwriter.h"

using namespace std;

SCENARIO("VMWriter Basics")
{
  test::MockReader R;

  SECTION("Simple tree dfs traversal")
  {
    strcpy(R.buffer, JACK_SEVEN_SRC);
    JackTokenizer Tokenizer(R);
    auto tokens = Tokenizer.parse_tokens();

    ParseTree T(ParseTreeNodeType_t::P_UNDEFINED, tokens);
    auto parsetree_node = T.parse_class();
    REQUIRE(parsetree_node);

    VmWriter VM(parsetree_node);
    stringstream ss;

    VM.unvisited_nodes.push_back(VM.pClassRootNode);

    while (auto node = VM.visit())
      ss << ParseTreeNode::to_string(node->type) << " ";

    // verify tree structure using strings
    REQUIRE(
        ss.str() ==

        "P_CLASS_DECL_BLOCK P_KEYWORD P_CLASS_NAME P_DELIMITER "
        "P_SUBROUTINE_DECL_BLOCK P_SUBROUTINE_TYPE P_RETURN_TYPE "
        "P_SUBROUTINE_NAME P_DELIMITER P_PARAMETER_LIST P_DELIMITER "
        "P_SUBROUTINE_BODY P_DELIMITER P_STATEMENT_LIST P_DO_STATEMENT "
        "P_KEYWORD P_SUBROUTINE_CALL P_SUBROUTINE_CALL_SITE_BINDING "
        "P_CLASS_OR_VAR_NAME P_DELIMITER P_SUBROUTINE_NAME P_DELIMITER "
        "P_EXPRESSION_LIST P_EXPRESSION P_TERM P_INTEGER_CONSTANT P_OP P_TERM "
        "P_DELIMITER P_EXPRESSION P_TERM P_INTEGER_CONSTANT P_OP P_TERM "
        "P_INTEGER_CONSTANT P_DELIMITER P_DELIMITER P_DELIMITER "
        "P_RETURN_STATEMENT P_KEYWORD P_DELIMITER P_DELIMITER P_DELIMITER ");
  }

  SECTION("Class vars")
  {
    strcpy(R.buffer, CLASSVAR_SRC);
    JackTokenizer Tokenizer(R);
    auto tokens = Tokenizer.parse_tokens();

    ParseTree T(ParseTreeNodeType_t::P_UNDEFINED, tokens);
    auto parsetree_node = T.parse_class();
    REQUIRE(parsetree_node);

    VmWriter VM(parsetree_node);
    stringstream ss;

    VM.lower_class();
    REQUIRE(VM.class_name == "testjack");

    const auto& ST = VM.get_symbol_table();

    const auto* s1 = ST.find_symbol("var1");
    REQUIRE(s1->name == "var1");
    REQUIRE(s1->var_index == 0);
    REQUIRE(get_if<Symbol::BasicType_t>(&s1->type));
    REQUIRE(get_if<Symbol::ClassStorageClass_t>(&s1->storage_class));
    REQUIRE(get<Symbol::BasicType_t>(s1->type) == Symbol::BasicType_t::T_INT);
    REQUIRE(get<Symbol::ClassStorageClass_t>(s1->storage_class) ==
            Symbol::ClassStorageClass_t::S_FIELD);

    const auto* s2 = ST.find_symbol("inst1");
    REQUIRE(s2->name == "inst1");
    REQUIRE(s2->var_index == 0);
    REQUIRE(get_if<Symbol::ClassType_t>(&s2->type));
    REQUIRE(get_if<Symbol::ClassStorageClass_t>(&s2->storage_class));
    REQUIRE(get<Symbol::ClassType_t>(s2->type) == "ClassName");
    REQUIRE(get<Symbol::ClassStorageClass_t>(s2->storage_class) ==
            Symbol::ClassStorageClass_t::S_STATIC);

    const auto* s3 = ST.find_symbol("var2");
    REQUIRE(s3->name == "var2");
    REQUIRE(s3->var_index == 1);
    REQUIRE(get_if<Symbol::BasicType_t>(&s3->type));
    REQUIRE(get_if<Symbol::ClassStorageClass_t>(&s3->storage_class));
    REQUIRE(get<Symbol::BasicType_t>(s3->type) ==
            Symbol::BasicType_t::T_BOOLEAN);
    REQUIRE(get<Symbol::ClassStorageClass_t>(s3->storage_class) ==
            Symbol::ClassStorageClass_t::S_FIELD);
  }

  SECTION("classname to file mismatch")
  {
    strcpy(R.buffer, CLASSVAR_SRC);
    JackTokenizer Tokenizer(R);
    auto tokens = Tokenizer.parse_tokens();

    ParseTree T(tokens, "NotTheSame");
    REQUIRE_THROWS(T.parse_class());
  }

  SECTION("Subroutine vars")
  {
    strcpy(R.buffer, CLASSANDSUBVARS_SRC);
    JackTokenizer Tokenizer(R);
    auto tokens = Tokenizer.parse_tokens();

    ParseTree T(ParseTreeNodeType_t::P_UNDEFINED, tokens);
    auto parsetree_node = T.parse_class();
    REQUIRE(parsetree_node);

    VmWriter VM(parsetree_node);
    stringstream ss;

    VM.lower_class();
    REQUIRE(VM.class_name == "JackTest");

    const auto& ST = VM.get_symbol_table();

    const auto* s0 = ST.find_symbol("this");
    REQUIRE(s0->name == "this");
    REQUIRE(s0->var_index == 0);
    REQUIRE(get_if<Symbol::ClassType_t>(&s0->type));
    REQUIRE(get_if<Symbol::SubroutineStorageClass_t>(&s0->storage_class));
    REQUIRE(get<Symbol::ClassType_t>(s0->type) == "JackTest");
    REQUIRE(get<Symbol::SubroutineStorageClass_t>(s0->storage_class) ==
            Symbol::SubroutineStorageClass_t::S_ARGUMENT);

    const auto* s1 = ST.find_symbol("a1");
    REQUIRE(s1->name == "a1");
    REQUIRE(s1->var_index == 1);
    REQUIRE(get_if<Symbol::BasicType_t>(&s1->type));
    REQUIRE(get_if<Symbol::SubroutineStorageClass_t>(&s1->storage_class));
    REQUIRE(get<Symbol::BasicType_t>(s1->type) == Symbol::BasicType_t::T_INT);
    REQUIRE(get<Symbol::SubroutineStorageClass_t>(s1->storage_class) ==
            Symbol::SubroutineStorageClass_t::S_ARGUMENT);

    const auto* s2 = ST.find_symbol("a2");
    REQUIRE(s2->name == "a2");
    REQUIRE(s2->var_index == 2);
    REQUIRE(get_if<Symbol::BasicType_t>(&s2->type));
    REQUIRE(get_if<Symbol::SubroutineStorageClass_t>(&s2->storage_class));
    REQUIRE(get<Symbol::BasicType_t>(s2->type) == Symbol::BasicType_t::T_INT);
    REQUIRE(get<Symbol::SubroutineStorageClass_t>(s2->storage_class) ==
            Symbol::SubroutineStorageClass_t::S_ARGUMENT);

    const auto* s3 = ST.find_symbol("v1");
    REQUIRE(s3->name == "v1");
    REQUIRE(s3->var_index == 0);
    REQUIRE(get_if<Symbol::BasicType_t>(&s3->type));
    REQUIRE(get_if<Symbol::SubroutineStorageClass_t>(&s3->storage_class));
    REQUIRE(get<Symbol::BasicType_t>(s3->type) == Symbol::BasicType_t::T_INT);
    REQUIRE(get<Symbol::SubroutineStorageClass_t>(s3->storage_class) ==
            Symbol::SubroutineStorageClass_t::S_LOCAL);

    const auto* s4 = ST.find_symbol("v2");
    REQUIRE(s4->name == "v2");
    REQUIRE(s4->var_index == 1);
    REQUIRE(get_if<Symbol::BasicType_t>(&s4->type));
    REQUIRE(get_if<Symbol::SubroutineStorageClass_t>(&s4->storage_class));
    REQUIRE(get<Symbol::BasicType_t>(s4->type) ==
            Symbol::BasicType_t::T_BOOLEAN);
    REQUIRE(get<Symbol::SubroutineStorageClass_t>(s4->storage_class) ==
            Symbol::SubroutineStorageClass_t::S_LOCAL);

    const auto* s5 = ST.find_symbol("v3");
    REQUIRE(s5->name == "v3");
    REQUIRE(s5->var_index == 2);
    REQUIRE(get_if<Symbol::BasicType_t>(&s5->type));
    REQUIRE(get_if<Symbol::SubroutineStorageClass_t>(&s5->storage_class));
    REQUIRE(get<Symbol::BasicType_t>(s5->type) ==
            Symbol::BasicType_t::T_BOOLEAN);
    REQUIRE(get<Symbol::SubroutineStorageClass_t>(s5->storage_class) ==
            Symbol::SubroutineStorageClass_t::S_LOCAL);
  }

  SECTION("Two subroutines vars")
  {
    strcpy(R.buffer, TWO_SUBS_SRC);
    JackTokenizer Tokenizer(R);
    auto tokens = Tokenizer.parse_tokens();

    ParseTree T(ParseTreeNodeType_t::P_UNDEFINED, tokens);
    auto parsetree_node = T.parse_class();
    REQUIRE(parsetree_node);

    VmWriter VM(parsetree_node);
    stringstream ss;

    VM.lower_class();
    REQUIRE(VM.class_name == "JackTest2");

    const auto& ST = VM.get_symbol_table();

    const auto* s1 = ST.find_symbol("v1");
    REQUIRE(s1 == nullptr);

    const auto* s2 = ST.find_symbol("v2");
    REQUIRE(s2->name == "v2");
    REQUIRE(s2->var_index == 0);
    REQUIRE(get_if<Symbol::BasicType_t>(&s2->type));
    REQUIRE(get_if<Symbol::SubroutineStorageClass_t>(&s2->storage_class));
    REQUIRE(get<Symbol::BasicType_t>(s2->type) == Symbol::BasicType_t::T_CHAR);
    REQUIRE(get<Symbol::SubroutineStorageClass_t>(s2->storage_class) ==
            Symbol::SubroutineStorageClass_t::S_LOCAL);
  }

  SECTION("Subroutine Descr")
  {
    strcpy(R.buffer, CLASSANDSUBVARS_SRC);
    JackTokenizer Tokenizer(R);
    auto tokens = Tokenizer.parse_tokens();

    ParseTree T(ParseTreeNodeType_t::P_UNDEFINED, tokens);
    auto parsetree_node = T.parse_class();
    REQUIRE(parsetree_node);

    VmWriter VM(parsetree_node);

    auto pClassNode =
        dynamic_cast<const ParseTreeNonTerminal*>(&(*parsetree_node));
    auto pSubroutineDeclNode = VM.find_first_nonterm_node(
        ParseTreeNodeType_t::P_SUBROUTINE_DECL_BLOCK, pClassNode);
    REQUIRE(pSubroutineDeclNode);

    auto descr = SubroutineDescr(VM, pSubroutineDeclNode);
    REQUIRE(descr.type == TokenValue_t::J_METHOD);
    REQUIRE(get_if<Symbol::BasicType_t>(&descr.return_type));
    REQUIRE(get<Symbol::BasicType_t>(descr.return_type) ==
            Symbol::BasicType_t::T_VOID);
    REQUIRE(descr.name == "sub1");
    REQUIRE(descr.pBody);
  }
}

SCENARIO("VMWriter Statements")
{
  test::MockReader R;

  SECTION("Single return statement")
  {
    strcpy(R.buffer, SINGLE_RETURN_SRC);

    JackTokenizer Tokenizer(R);
    auto tokens = Tokenizer.parse_tokens();

    ParseTree T(ParseTreeNodeType_t::P_UNDEFINED, tokens);
    auto parsetree_node = T.parse_class();
    REQUIRE(parsetree_node);

    VmWriter VM(parsetree_node);
    VM.lower_class();

    REQUIRE(VM.class_name == "Main");

    REQUIRE(VM.lowered_vm.str() ==
            "function Main.f1 0\n"
            "push constant 1\n"
            "return\n");
  }

  SECTION("Compile Seven test program")
  {
    strcpy(R.buffer, JACK_SEVEN_SRC);

    JackTokenizer Tokenizer(R);
    auto tokens = Tokenizer.parse_tokens();

    ParseTree T(ParseTreeNodeType_t::P_UNDEFINED, tokens);
    auto parsetree_node = T.parse_class();
    REQUIRE(parsetree_node);

    VmWriter VM(parsetree_node);
    VM.lower_class();

    REQUIRE(VM.class_name == "Main");

    REQUIRE(VM.lowered_vm.str() ==
            "function Main.main 0\n"
            "push constant 1\n"
            "push constant 2\n"
            "push constant 3\n"
            "call Math.multiply 2\n"
            "add\n"
            "call Output.printInt 1\n"
            "pop temp 0\n"
            "push constant 0\n"
            "return\n");
  }

  SECTION("Let statement")
  {
    strcpy(R.buffer, LET_STATEMENT_SRC);

    JackTokenizer Tokenizer(R);
    auto tokens = Tokenizer.parse_tokens();

    ParseTree T(ParseTreeNodeType_t::P_UNDEFINED, tokens);
    auto parsetree_node = T.parse_class();
    REQUIRE(parsetree_node);

    VmWriter VM(parsetree_node);
    VM.lower_class();

    REQUIRE(VM.class_name == "Main");

    REQUIRE(VM.lowered_vm.str() ==
            "function Main.f1 3\n"
            "push constant 1\n"
            "pop local 0\n"
            "push constant 1\n"
            "push local 1\n"
            "neg\n"
            "add\n"
            "pop local 1\n"
            "push constant 0\n"
            "not\n"
            "pop local 2\n"
            "push local 0\n"
            "push local 1\n"
            "push constant 2\n"
            "call Math.pow 2\n"
            "add\n"
            "return\n");
  }

  SECTION("Class method call")
  {
    strcpy(R.buffer, CLASS_METHOD_CALL_SRC);

    JackTokenizer Tokenizer(R);
    auto tokens = Tokenizer.parse_tokens();

    ParseTree T(ParseTreeNodeType_t::P_UNDEFINED, tokens);
    auto parsetree_node = T.parse_class();
    REQUIRE(parsetree_node);

    VmWriter VM(parsetree_node);
    VM.lower_class();

    REQUIRE(VM.class_name == "Main");

    REQUIRE(VM.lowered_vm.str() ==
            "function Main.mul 0\n"
            "push argument 0\n"
            "pop pointer 0\n"
            "push constant 0\n"
            "return\n"
            "function Main.f1 0\n"
            "push argument 0\n"
            "pop pointer 0\n"
            "push pointer 0\n"
            "push constant 1\n"
            "push constant 2\n"
            "call Main.mul 3\n"
            "return\n");
  }

  SECTION("IF-ELSE structure")
  {
    strcpy(R.buffer, SIMPLE_IF_SRC);

    JackTokenizer Tokenizer(R);
    auto tokens = Tokenizer.parse_tokens();

    ParseTree T(ParseTreeNodeType_t::P_UNDEFINED, tokens);
    auto parsetree_node = T.parse_class();
    REQUIRE(parsetree_node);

    VmWriter VM(parsetree_node);
    VM.lower_class();

    REQUIRE(VM.class_name == "IfTest");

    REQUIRE(VM.lowered_vm.str() ==
            "function IfTest.f1 1\n"
            "push argument 0\n"
            "push constant 50\n"
            "gt\n"
            "not\n"
            "if-goto IfTest.f1.0.IF_FALSE\n"
            "push constant 1\n"
            "pop local 0\n"
            "goto IfTest.f1.0.IF_END\n"
            "label IfTest.f1.0.IF_FALSE\n"
            "push argument 0\n"
            "pop local 0\n"
            "label IfTest.f1.0.IF_END\n"
            "push local 0\n"
            "return\n");
  }

  SECTION("WHILE structure")
  {
    strcpy(R.buffer, SIMPLE_WHILE_SRC);

    JackTokenizer Tokenizer(R);
    auto tokens = Tokenizer.parse_tokens();

    ParseTree T(ParseTreeNodeType_t::P_UNDEFINED, tokens);
    auto parsetree_node = T.parse_class();
    REQUIRE(parsetree_node);

    VmWriter VM(parsetree_node);
    VM.lower_class();

    REQUIRE(VM.class_name == "WhileTest");

    REQUIRE(VM.lowered_vm.str() ==
            "function WhileTest.f1 0\n"
            "label WhileTest.f1.0.WHILE_BEGIN\n"
            "push argument 0\n"
            "push constant 0\n"
            "gt\n"
            "not\n"
            "if-goto WhileTest.f1.0.WHILE_END\n"
            "push argument 0\n"
            "push constant 1\n"
            "sub\n"
            "pop argument 0\n"
            "goto WhileTest.f1.0.WHILE_BEGIN\n"
            "label WhileTest.f1.0.WHILE_END\n"
            "push argument 0\n"
            "return\n");
  }

  SECTION("Somple Constructor")
  {
    strcpy(R.buffer, SIMPLE_CONST_SRC);

    JackTokenizer Tokenizer(R);
    auto tokens = Tokenizer.parse_tokens();

    ParseTree T(ParseTreeNodeType_t::P_UNDEFINED, tokens);
    auto parsetree_node = T.parse_class();
    REQUIRE(parsetree_node);

    VmWriter VM(parsetree_node);
    VM.lower_class();

    REQUIRE(VM.class_name == "Test");

    REQUIRE(VM.lowered_vm.str() ==
            "function Test.new 0\n"
            "push constant 0\n"
            "call Memory.alloc 1\n"
            "pop pointer 0\n"
            "push pointer 0\n"
            "return\n"
            "function Test.ref 0\n"
            "push argument 0\n"
            "pop pointer 0\n"
            "push pointer 0\n"
            "return\n");
  }

  SECTION("Void Method return")
  {
    strcpy(R.buffer, VOID_RETURN_SRC);

    JackTokenizer Tokenizer(R);
    auto tokens = Tokenizer.parse_tokens();

    ParseTree T(ParseTreeNodeType_t::P_UNDEFINED, tokens);
    auto parsetree_node = T.parse_class();
    REQUIRE(parsetree_node);

    VmWriter VM(parsetree_node);
    VM.lower_class();

    REQUIRE(VM.class_name == "Test");

    REQUIRE(VM.lowered_vm.str() ==
            "function Test.f1 0\n"
            "push argument 0\n"
            "pop pointer 0\n"
            "push constant 0\n"
            "return\n");
  }

  SECTION("Method Call From Constructor")
  {
    strcpy(R.buffer, OBJECT_METHOD_CALL_SRC);

    JackTokenizer Tokenizer(R);
    auto tokens = Tokenizer.parse_tokens();

    ParseTree T(ParseTreeNodeType_t::P_UNDEFINED, tokens);
    auto parsetree_node = T.parse_class();
    REQUIRE(parsetree_node);

    VmWriter VM(parsetree_node);
    VM.lower_class();

    REQUIRE(VM.class_name == "Test");

    REQUIRE(VM.lowered_vm.str() ==
            "function Test.main 1\n"
            "call MyClass.new 0\n"
            "pop local 0\n"
            "push local 0\n"
            "call MyClass.run 1\n"
            "pop temp 0\n"
            "push constant 0\n"
            "return\n");
  }

  SECTION("Method Call From Constructor")
  {
    strcpy(R.buffer, CONST_METHOD_CALL_SRC);

    JackTokenizer Tokenizer(R);
    auto tokens = Tokenizer.parse_tokens();

    ParseTree T(ParseTreeNodeType_t::P_UNDEFINED, tokens);
    auto parsetree_node = T.parse_class();
    REQUIRE(parsetree_node);

    VmWriter VM(parsetree_node);
    VM.lower_class();

    REQUIRE(VM.class_name == "Test");

    REQUIRE(VM.lowered_vm.str() ==
            "function Test.new 0\n"
            "push constant 1\n"
            "call Memory.alloc 1\n"
            "pop pointer 0\n"
            "push pointer 0\n"
            "call Test.draw 1\n"
            "pop temp 0\n"
            "push pointer 0\n"
            "return\n"
            "function Test.draw 0\n"
            "push argument 0\n"
            "pop pointer 0\n"
            "push constant 0\n"
            "return\n");
  }

#if 0
  SECTION("LHS Array Assignment")
  {
    strcpy(R.buffer, LHS_ARRAY_ASSIGN_SRC);

    JackTokenizer Tokenizer(R);
    auto tokens = Tokenizer.parse_tokens();

    ParseTree T(ParseTreeNodeType_t::P_UNDEFINED, tokens);
    auto parsetree_node = T.parse_class();
    REQUIRE(parsetree_node);

    VmWriter VM(parsetree_node);
    VM.lower_class();

    REQUIRE(VM.class_name == "Test");

    REQUIRE(VM.lowered_vm.str() ==
            "function Test.new 2\n"
            "push constant 5\n"
            "pop local 1\n"       // b = 5
            "push constant 3\n"
            "call Array.new 1\n"
            "pop local 0\n"       // a = Array.new(3)
            "push local 1\n"      // b
            "push local 0\n"      // &a
            "pop constant 2\n"
            "add\n"               // a[2]
            "pop pointer 1\n"
            "pop that 0\n"        // b -> a[2]
            "push constant 0\n"
            "return\n");
  }
#endif

#if 0
  SECTION("RHS Array Assignment")
  {
    strcpy(R.buffer, RHS_ARRAY_ASSIGN_SRC);

    JackTokenizer Tokenizer(R);
    auto tokens = Tokenizer.parse_tokens();

    ParseTree T(ParseTreeNodeType_t::P_UNDEFINED, tokens);
    auto parsetree_node = T.parse_class();
    REQUIRE(parsetree_node);

    VmWriter VM(parsetree_node);
    VM.lower_class();

    REQUIRE(VM.class_name == "Test");

    REQUIRE(VM.lowered_vm.str() ==
            "function Test.new 1\n"
            "push constant 2\n"
            "call Array.new 1\n"
            "pop local 0\n"
            //
            // let a[1] = 5
            "push constant 5\n"
            // a[1] <-- 5
            "push constant 1\n"
            "push local 0\n"
            "add\n"
            "pop pointer 1\n"
            "pop that 0\n"
            //
            // let a[0] = 6
            "push constant 6\n"
            // a[0] <-- 6
            "push constant 0\n"
            "push local 0\n"
            "add\n"
            "pop pointer 1\n"
            "pop that 0\n"
            //
            // a[0]
            "push constant 0\n"
            "push local 0\n"
            "add\n"
            "pop pointer 1\n"
            "push that 0\n"
            "return\n");
  }
#endif

#if 0
  SECTION("Array-Array assignment")
  {
    strcpy(R.buffer, ARRAY_ARRAY_ASSIGN_SRC);

    JackTokenizer Tokenizer(R);
    auto tokens = Tokenizer.parse_tokens();

    ParseTree T(ParseTreeNodeType_t::P_UNDEFINED, tokens);
    auto parsetree_node = T.parse_class();
    REQUIRE(parsetree_node);

    VmWriter VM(parsetree_node);
    VM.lower_class();

    REQUIRE(VM.class_name == "Test");

    REQUIRE(VM.lowered_vm.str() ==
            "function Test.main 2\n"
            "push constant 1\n"
            "call Array.new 1\n"
            "pop local 0\n"
            "push constant 1\n"
            "call Array.new 1\n"
            "pop local 1\n"
            "push constant 0\n"
            "push local 1\n"
            "add\n"
            "push constant 5\n"
            "pop temp 0\n"
            "pop pointer 1\n"
            "push temp 0\n"
            "pop that 0\n"
            "push constant 0\n"
            "push local 0\n"
            "add\n"
            "push constant 0\n"
            "push local 1\n"
            "add\n"
            "pop pointer 1\n"
            "push that 0\n"
            "pop temp 0\n"
            "pop pointer 1\n"
            "push temp 0\n"
            "pop that 0\n"
            "push constant 0\n"
            "return\n");
  }
#endif
}
