#include "jack_sources.h"

const char* SINGLE_RETURN_SRC = R"""(
class Main {
    function int f1() {
        return 1;
    }
}
)""";

const char* VOID_RETURN_SRC = R"""(
class Test {
   method void f1() {
      return;
   }
}
)""";

const char* SIMPLE_CONST_SRC = R"""(
class Test {
   constructor Test new() {
      return this;
   }

   method int ref() {
      return this;
   }
}
)""";

const char* CONST_VOID_METHOD_CALL_SRC = R"""(
class Test {
   method void draw() {
      return;
   }

   method void run() {
      do draw();
      return;
   }
}
)""";

const char* CONST_VOID_METHOD_CALL_GLOBAL_SRC = R"""(
class Test {
   method void draw(bool a, bool b) {
      do Screen.setColor(a & b);
      return;
   }

   method void run() {
      do draw(true, true);
      return;
   }
}
)""";

// TODO: Work through different call examples
// Calls to methods/functions/constructor
// from method/function/contructor

const char* CONSTRUCTOR_CALL_GLOBAL_SRC = R"""(
class Wrapper {
   constructor Wrapper new(int a) {
      do Array.new(a);
      return this;
   }
}
)""";

const char* CLASSVAR_SRC = R"""(
class testjack {
    static ClassName inst1;
    field int var1;
    field boolean var2;
    static char var3, var4;
}
)""";

const char* CLASSVARS_AND_SUBVARS_SRC = R"""(
class JackTest {
    static ClassName inst1;
    field int var1;

    method void sub1(int a1, int a2) {
        var int v1;
        var boolean v2, v3;

        return;
    }
}
)""";

const char* SINGLE_RETURN_EXPRESSIONS_SRC = R"""(
class MathExp {
    function int f1() {
        return (((1 + 2) + 3) + 4) + 5;
    }

    function int f2() {
        return (1 + (2 / 3)) - (4 * 5);
    }

    function int f3() {
        return -2;
    }

    function int f4() {
        return ~3;
    }

    function bool f5() {
        return (true | (true & true));
    }

    function bool f6() {
        return 5 = 6;
    }

    function bool f7() {
        return ~true;
    }

    function bool f8() {
        return (2 > 1) & (1 < 2);
    }
}
)""";

// Prints 14 twice
const char* STATIC_CLASS_SRC = R"""(
class Main {
  function int Res() {
     do Output.printInt(Main.Res());
     return Math.multiply(2, 7);
  }

  function int main() {
     return Math.Res();
  }
}
)""";

const char* SIMPLE_WHILE_SRC = R"""(
class WhileTest {
    function int f1(int a) {
        while (a > 0)
        {
            let a = a - 1;
        }

        return a;
    }
}
)""";

const char* SIMPLE_IF_SRC = R"""(
class IfTest {
    function int f1(int a) {
        var int r;

        if (a > 50)
        {
            let r = 2;
        }
        else
        {
            let r = a;
        }

        if (a > 60)
        {
            let r = 1;
        }

        return r;
    }
}
)""";
