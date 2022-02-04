#include <string.h>

// for testing
#include <iostream>

#include "common.h"
#include "jack_sources.h"
#include "vmwriter/vmwriter.h"

SCENARIO("VMWriter Statements")
{
  SECTION("Single return statement")
  {
    TextReader R(SINGLE_RETURN_SRC);

    JackTokenizer T(R);
    auto tokens = T.parse_tokens();
    recursive_descent::Parser parser(tokens);
    parser.parse_class();

    VmWriter::VmWriter VM(parser.get_ast());
    // VM.dump_ast();
    VM.lower_module();

    REQUIRE(VM.get_lowered_vm() ==
            "function Main.f1 0\n"
            "push constant 1\n"
            "return\n");
  }

  SECTION("Single return expressions")
  {
    TextReader R(SINGLE_RETURN_EXPRESSIONS_SRC);

    JackTokenizer T(R);
    auto tokens = T.parse_tokens();
    recursive_descent::Parser parser(tokens);
    parser.parse_class();

    VmWriter::VmWriter VM(parser.get_ast());
    // VM.dump_ast();
    VM.lower_module();

    REQUIRE(VM.get_lowered_vm() ==
            "function MathExp.f1 0\n"
            "push constant 1\n"
            "push constant 2\n"
            "add\n"
            "push constant 3\n"
            "add\n"
            "push constant 4\n"
            "add\n"
            "push constant 5\n"
            "add\n"
            "return\n"
            "function MathExp.f2 0\n"
            "push constant 1\n"
            "push constant 2\n"
            "push constant 3\n"
            "call Math.divide 2\n"
            "add\n"
            "push constant 4\n"
            "push constant 5\n"
            "call Math.multiply 2\n"
            "sub\n"
            "return\n"
            "function MathExp.f3 0\n"
            "push constant 2\n"
            "neg\n"
            "return\n"
            "function MathExp.f4 0\n"
            "push constant 3\n"
            "not\n"
            "return\n"
            "function MathExp.f5 0\n"
            "push constant 0\n"
            "not\n"
            "push constant 0\n"
            "not\n"
            "push constant 0\n"
            "not\n"
            "and\n"
            "or\n"
            "return\n"
            "function MathExp.f6 0\n"
            "push constant 5\n"
            "push constant 6\n"
            "eq\n"
            "return\n"
            "function MathExp.f7 0\n"
            "push constant 0\n"
            "not\n"
            "not\n"
            "return\n"
            "function MathExp.f8 0\n"
            "push constant 2\n"
            "push constant 1\n"
            "gt\n"
            "push constant 1\n"
            "push constant 2\n"
            "lt\n"
            "and\n"
            "return\n");
  }

  SECTION("Void Method return")
  {
    TextReader R(VOID_RETURN_SRC);

    JackTokenizer T(R);
    auto tokens = T.parse_tokens();
    recursive_descent::Parser parser(tokens);
    parser.parse_class();

    VmWriter::VmWriter VM(parser.get_ast());
    // VM.dump_ast();
    VM.lower_module();

    REQUIRE(VM.get_lowered_vm() ==
            "function Test.f1 0\n"
            "push argument 0\n"
            "pop pointer 0\n"
            "push constant 0\n"
            "return\n");
  }

  SECTION("Simple Constructor")
  {
    TextReader R(SIMPLE_CONST_SRC);

    JackTokenizer T(R);
    auto tokens = T.parse_tokens();
    recursive_descent::Parser parser(tokens);
    parser.parse_class();

    VmWriter::VmWriter VM(parser.get_ast());
    // VM.dump_ast();
    VM.lower_module();

    REQUIRE(VM.get_lowered_vm() ==
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

  SECTION("Local method call")
  {
    TextReader R(CONST_VOID_METHOD_CALL_SRC);

    JackTokenizer T(R);
    auto tokens = T.parse_tokens();
    recursive_descent::Parser parser(tokens);
    parser.parse_class();

    VmWriter::VmWriter VM(parser.get_ast());
    // VM.dump_ast();
    VM.lower_module();

    REQUIRE(VM.get_lowered_vm() ==
            "function Test.draw 0\n"
            "push argument 0\n"
            "pop pointer 0\n"
            "push constant 0\n"
            "return\n"
            "function Test.run 0\n"
            "push argument 0\n"
            "pop pointer 0\n"
            "push pointer 0\n"
            "call Test.draw 1\n"
            "pop temp 0\n"
            "push constant 0\n"
            "return\n");
  }

  SECTION("Global method call w/parms")
  {
    TextReader R(CONST_VOID_METHOD_CALL_GLOBAL_SRC);

    JackTokenizer T(R);
    auto tokens = T.parse_tokens();
    recursive_descent::Parser parser(tokens);
    parser.parse_class();

    VmWriter::VmWriter VM(parser.get_ast());
    // VM.dump_ast();
    VM.lower_module();

    REQUIRE(VM.get_lowered_vm() ==
            "function Test.draw 0\n"
            "push argument 0\n"
            "pop pointer 0\n"
            "push argument 1\n"
            "push argument 2\n"
            "and\n"
            "call Screen.setColor 1\n"
            "pop temp 0\n"
            "push constant 0\n"
            "return\n"
            "function Test.run 0\n"
            "push argument 0\n"
            "pop pointer 0\n"
            "push pointer 0\n"
            "push constant 0\n"
            "not\n"
            "push constant 0\n"
            "not\n"
            "call Test.draw 3\n"
            "pop temp 0\n"
            "push constant 0\n"
            "return\n");
  }

  SECTION("Object global method call")
  {
    Expected_t program_in = {
        ""  // clang-format sorcery
        "class Test {",
        "   field Square square;",
        " ",
        "   method void dispose() {",
        "      do square.check(1);",
        "      do square.dispose();",
        "      return;",
        "   }",
        "}",
        ""};
    std::string expected_str = expected_string(program_in);
    TextReader R(expected_str.data());

    JackTokenizer T(R);
    auto tokens = T.parse_tokens();
    recursive_descent::Parser parser(tokens);
    parser.parse_class();

    VmWriter::VmWriter VM(parser.get_ast());
    // VM.dump_ast();
    VM.lower_module();

    REQUIRE(VM.get_lowered_vm() ==
            "function Test.dispose 0\n"
            "push argument 0\n"
            "pop pointer 0\n"
            "push this 0\n"
            "push constant 1\n"
            "call Square.check 2\n"
            "pop temp 0\n"
            "push this 0\n"
            "call Square.dispose 1\n"
            "pop temp 0\n"
            "push constant 0\n"
            "return\n");
  }

  SECTION("Global function as call argument, VM")
  {
    TextReader R(STATIC_CLASS_SRC);

    JackTokenizer T(R);
    auto tokens = T.parse_tokens();
    recursive_descent::Parser parser(tokens);
    parser.parse_class();

    VmWriter::VmWriter VM(parser.get_ast());
    // VM.dump_ast();
    VM.lower_module();

    REQUIRE(VM.get_lowered_vm() ==
            "function Main.Res 0\n"
            "call Main.Res 0\n"
            "call Output.printInt 1\n"
            "pop temp 0\n"
            "push constant 2\n"
            "push constant 7\n"
            "call Math.multiply 2\n"
            "return\n"
            "function Main.main 0\n"
            "call Math.Res 0\n"
            "return\n");
  }

  SECTION("Valid static class")
  {
    Expected_t program_in = {
        ""  // clang-format sorcery
        "class Main {",
        "   function void main() {",
        "      do Output.printInt(1 + (2 * 3));",
        "      return;",
        "   }",
        "}",
        ""};
    std::string expected_str = expected_string(program_in);
    TextReader R(expected_str.data());

    JackTokenizer T(R);
    auto tokens = T.parse_tokens();
    recursive_descent::Parser parser(tokens);
    parser.parse_class();

    VmWriter::VmWriter VM(parser.get_ast());
    // VM.dump_ast();
    VM.lower_module();

    REQUIRE(VM.get_lowered_vm() ==
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

  SECTION("Vars check")
  {
    Expected_t program_in = {
        ""  // clang-format sorcery
        "class Test {",
        "    static ClassName inst1;",
        "    field int var1;",
        "    field boolean var2;",
        " ",
        "    method void sub1(int a1, int a2) {",
        "        var int v1;",
        "        var boolean v2, v3;",
        " ",
        "        return;",
        "    }",
        "}"};
    std::string expected_str = expected_string(program_in);
    TextReader R(expected_str.data());

    JackTokenizer T(R);
    auto tokens = T.parse_tokens();
    recursive_descent::Parser parser(tokens);
    parser.parse_class();

    VmWriter::VmWriter VM(parser.get_ast());
    // VM.dump_ast();
    VM.lower_module();

    REQUIRE(VM.get_lowered_vm() ==
            "function Test.sub1 3\n"
            "push argument 0\n"
            "pop pointer 0\n"
            "push constant 0\n"
            "return\n"
            "");
  }

  SECTION("Let statement")
  {
    Expected_t program_in = {
        ""  // clang-format sorcery
        "class Main {",
        "    function int f1() {",
        "        var int v1, v2;",
        "        var bool v3;",
        " ",
        "        let v1 = 1;",
        "        let v2 = 1 + -v2;",
        "        let v3 = true;",
        " ",
        "        return v1 + Math.pow(v2, 2);",
        "    }",
        "}",
        ""};
    std::string expected_str = expected_string(program_in);
    TextReader R(expected_str.data());

    JackTokenizer T(R);
    auto tokens = T.parse_tokens();
    recursive_descent::Parser parser(tokens);
    parser.parse_class();

    VmWriter::VmWriter VM(parser.get_ast());
    // VM.dump_ast();
    VM.lower_module();

    REQUIRE(VM.get_lowered_vm() ==
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

  SECTION("Object method call")
  {
    Expected_t program_in = {
        ""  // clang-format sorcery
        "class Test {",
        "    function void main() {",
        "        var MyClass c;",
        "        let c = MyClass.new();",
        "        do c.run();",
        "        return;",
        "    }",
        "}",
        ""};
    std::string expected_str = expected_string(program_in);
    TextReader R(expected_str.data());

    JackTokenizer T(R);
    auto tokens = T.parse_tokens();
    recursive_descent::Parser parser(tokens);
    parser.parse_class();

    VmWriter::VmWriter VM(parser.get_ast());
    // VM.dump_ast();
    VM.lower_module();

    REQUIRE(VM.get_lowered_vm() ==
            "function Test.main 1\n"
            "call MyClass.new 0\n"
            "pop local 0\n"
            "push local 0\n"
            "call MyClass.run 1\n"
            "pop temp 0\n"
            "push constant 0\n"
            "return\n");
  }

  SECTION("Local method call from constructor")
  {
    Expected_t program_in = {
        ""  // clang-format sorcery
        "class Test {",
        "   field int a;",
        " ",
        "   constructor Test new() {",
        "      do draw();",
        "      return this;",
        "   }",
        "   method void draw() {",
        "      return;",
        "   }",
        "}",
        ""};
    std::string expected_str = expected_string(program_in);
    TextReader R(expected_str.data());

    JackTokenizer T(R);
    auto tokens = T.parse_tokens();
    recursive_descent::Parser parser(tokens);
    parser.parse_class();

    VmWriter::VmWriter VM(parser.get_ast());
    // VM.dump_ast();
    VM.lower_module();

    REQUIRE(VM.get_lowered_vm() ==
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

  SECTION("Simple while loop")
  {
    TextReader R(SIMPLE_WHILE_SRC);

    JackTokenizer T(R);
    auto tokens = T.parse_tokens();
    recursive_descent::Parser parser(tokens);
    parser.parse_class();

    VmWriter::VmWriter VM(parser.get_ast());
    // VM.dump_ast();
    VM.lower_module();

    REQUIRE(VM.get_lowered_vm() ==
            "function WhileTest.f1 0\n"
            "label WHILE_BEGIN_0\n"
            "push argument 0\n"
            "push constant 0\n"
            "gt\n"
            "if-goto WHILE_TRUE_0\n"
            "goto WHILE_END_0\n"
            "label WHILE_TRUE_0\n"
            "push argument 0\n"
            "push constant 1\n"
            "sub\n"
            "pop argument 0\n"
            "goto WHILE_BEGIN_0\n"
            "label WHILE_END_0\n"
            "push argument 0\n"
            "return\n");
  }

  SECTION("IF-ELSE structure")
  {
    TextReader R(SIMPLE_IF_SRC);

    JackTokenizer T(R);
    auto tokens = T.parse_tokens();
    recursive_descent::Parser parser(tokens);
    parser.parse_class();

    VmWriter::VmWriter VM(parser.get_ast());
    // VM.dump_ast();
    VM.lower_module();

    REQUIRE(VM.get_lowered_vm() ==
            "function IfTest.f1 1\n"
            "push argument 0\n"
            "push constant 50\n"
            "gt\n"
            "if-goto IF_TRUE_0\n"
            "label IF_FALSE_0\n"
            "push argument 0\n"
            "pop local 0\n"
            "goto IF_END_0\n"
            "label IF_TRUE_0\n"
            "push constant 2\n"
            "pop local 0\n"
            "label IF_END_0\n"
            "push argument 0\n"
            "push constant 60\n"
            "gt\n"
            "if-goto IF_TRUE_1\n"
            "goto IF_END_1\n"
            "label IF_TRUE_1\n"
            "push constant 1\n"
            "pop local 0\n"
            "label IF_END_1\n"
            "push local 0\n"
            "return\n");
  }

  SECTION("Numerical IF")
  {
    Expected_t program_in = {
        ""  // clang-format sorcery
        "class Main {",
        "  function void main() {",
        "    if (8191 & 2)",
        "    {",
        "      do Output.printInt(1);",
        "    }",
        "    else",
        "    {",
        "      do Output.printInt(255);",
        "    }",
        " ",
        "    return;",
        "  }",
        "}",
        "",
        ""};
    std::string expected_str = expected_string(program_in);
    TextReader R(expected_str.data());

    JackTokenizer T(R);
    auto tokens = T.parse_tokens();
    recursive_descent::Parser parser(tokens);
    parser.parse_class();

    VmWriter::VmWriter VM(parser.get_ast());
    // VM.dump_ast();
    VM.lower_module();

    REQUIRE(VM.get_lowered_vm() ==
            "function Main.main 0\n"
            "push constant 8191\n"
            "push constant 2\n"
            "and\n"
            "if-goto IF_TRUE_0\n"
            "label IF_FALSE_0\n"
            "push constant 255\n"
            "call Output.printInt 1\n"
            "pop temp 0\n"
            "goto IF_END_0\n"
            "label IF_TRUE_0\n"
            "push constant 1\n"
            "call Output.printInt 1\n"
            "pop temp 0\n"
            "label IF_END_0\n"
            "push constant 0\n"
            "return\n");
  }

  SECTION("LHS Array Assignment")
  {
    Expected_t program_in = {
        ""  // clang-format sorcery
        "class Test {",
        "   function void main() {",
        "     var Array a;",
        "     var int b;",
        "     let b = 5;",
        "     let a = Array.new(3);",
        "     let a[2] = b;",
        "     return;",
        "   }",
        "}",
        ""};
    std::string expected_str = expected_string(program_in);
    TextReader R(expected_str.data());

    JackTokenizer T(R);
    auto tokens = T.parse_tokens();
    recursive_descent::Parser parser(tokens);
    parser.parse_class();

    VmWriter::VmWriter VM(parser.get_ast());
    // VM.dump_ast();
    VM.lower_module();

    REQUIRE(VM.get_lowered_vm() ==
            // clang-format off
            "function Test.main 2\n"
            "push constant 5\n"
            "pop local 1\n"             // b = 5
            "push constant 3\n"
            "call Array.new 1\n"
            "pop local 0\n"             // a = Array.new(3)
            "push local 1\n"            // b
            "push constant 2\n"
            "push local 0\n"            // &a
            "add\n"                     // &a[2]
            "pop pointer 1\n"
            "pop that 0\n"              // b -> a[2]
            "push constant 0\n"
            "return\n"
            // clang-format on
    );
  }

  SECTION("RHS Array Assignment")
  {
    Expected_t program_in = {
        ""  // clang-format sorcery
        "class Test {",
        "   function int main() {",
        "     var Array a;",
        "     let a = Array.new(2);",
        "     let a[1] = 5;",
        "     let a[0] = 6;",
        "     return a[1];",
        "   }",
        "}",
        ""};
    std::string expected_str = expected_string(program_in);
    TextReader R(expected_str.data());

    JackTokenizer T(R);
    auto tokens = T.parse_tokens();
    recursive_descent::Parser parser(tokens);
    parser.parse_class();

    VmWriter::VmWriter VM(parser.get_ast());
    // VM.dump_ast();
    VM.lower_module();

    REQUIRE(VM.get_lowered_vm() ==
            "function Test.main 1\n"
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
            // a[1]
            "push constant 1\n"
            "push local 0\n"
            "add\n"
            "pop pointer 1\n"
            "push that 0\n"
            "return\n");
  }

  SECTION("Array-Array assignment")
  {
    Expected_t program_in = {
        ""  // clang-format sorcery
        "class Test {",
        "   function void main() {",
        "     var Array a;",
        "     var Array b;",
        "     let a = Array.new(1);",
        "     let b = Array.new(1);",
        "     let b[0] = 5;",
        "     let a[0] = b[0];",
        "     return;",
        "   }",
        "}",
        ""};
    std::string expected_str = expected_string(program_in);
    TextReader R(expected_str.data());

    JackTokenizer T(R);
    auto tokens = T.parse_tokens();
    recursive_descent::Parser parser(tokens);
    parser.parse_class();

    VmWriter::VmWriter VM(parser.get_ast());
    // VM.dump_ast();
    VM.lower_module();

    REQUIRE(VM.get_lowered_vm() ==
            "function Test.main 2\n"
            "push constant 1\n"
            "call Array.new 1\n"
            "pop local 0\n"
            "push constant 1\n"
            "call Array.new 1\n"
            "pop local 1\n"
            "push constant 5\n"
            "push constant 0\n"
            "push local 1\n"
            "add\n"
            "pop pointer 1\n"
            "pop that 0\n"
            "push constant 0\n"
            "push local 1\n"
            "add\n"
            "pop pointer 1\n"
            "push that 0\n"
            "push constant 0\n"
            "push local 0\n"
            "add\n"
            "pop pointer 1\n"
            "pop that 0\n"
            "push constant 0\n"
            "return\n");
  }

  SECTION("String term")
  {
    TextReader R(STRING_TERM_SRC);

    JackTokenizer T(R);
    auto tokens = T.parse_tokens();
    recursive_descent::Parser parser(tokens);
    parser.parse_class();

    VmWriter::VmWriter VM(parser.get_ast());
    // VM.dump_ast();
    VM.lower_module();

    REQUIRE(VM.get_lowered_vm() ==
            "function Test.main 0\n"
            "push constant 5\n"
            "call String.new 1\n"
            "push constant 72\n"
            "call String.appendChar 2\n"
            "push constant 101\n"
            "call String.appendChar 2\n"
            "push constant 108\n"
            "call String.appendChar 2\n"
            "push constant 108\n"
            "call String.appendChar 2\n"
            "push constant 111\n"
            "call String.appendChar 2\n"
            "call Output.printString 1\n"
            "pop temp 0\n"
            "push constant 0\n"
            "return\n");
  }
}
