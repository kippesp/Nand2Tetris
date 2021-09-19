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

const char* CONST_VOID_METHOD_CALL_SRC = R"""(
class Test {
   field int a;

   constructor Test new() {
      do draw();
      return this;
   }
   method void draw() {
      return;
   }
}
)""";

const char* CLASS_METHOD_CALL_SRC = R"""(
class Main {
    method int mul(int a, b) {
        return 0;
    }

    method int f1() {
        return mul(1, 2);
    }
}
)""";

const char* STRING_TERM_SRC = R"""(
class Test {
   function void main() {
     do Output.printString("Hello");
     return;
   }
}
)""";

const char* TWO_SUBS_SRC = R"""(
class JackTest2 {
    static ClassName inst1;
    field int var1;
    field boolean var2;

    function int s1() {
        var int v1;

        return 1;
    }

    function int s2() {
        var char v2;

        return 2;
    }
}
)""";

const char* OBJECT_METHOD_CALL_SRC = R"""(
class Test {
    function void main() {
        var MyClass c;
        let c = MyClass.new();
        do c.run();
        return;
    }
}
)""";

const char* SIMPLE_IF_SRC = R"""(
class IfTest {
    function int f1(int a) {
        var int r;

        if (a > 50)
        {
            let r = 1;
        }
        else
        {
            let r = a;
        }

        return r;
    }
}
)""";

const char* LHS_ARRAY_ASSIGN_SRC = R"""(
class Test {
   function void main() {
     var Array a;
     var int b;
     let b = 5;
     let a = Array.new(3);
     let a[2] = b;
     return;
   }
}
)""";

const char* RHS_ARRAY_ASSIGN_SRC = R"""(
class Test {
   function int main() {
     var Array a;
     let a = Array.new(2);
     let a[1] = 5;
     let a[0] = 6;
     return a[0];
   }
}
)""";

const char* ARRAY_ARRAY_ASSIGN_SRC = R"""(
class Test {
   function void main() {
     var Array a;
     var Array b;
     let a = Array.new(1);
     let b = Array.new(1);
     let b[0] = 5;
     let a[0] = b[0];
     return;
   }
}
)""";

const char* NUMERICAL_IF_SRC = R"""(
class Main {
  function void main() {
    if (8191 & 2)
    {
      do Output.printInt(1);
    }
    else
    {
      do Output.printInt(255);
    }

    return;
  }
}
)""";

const char* JACK_SEVEN_SRC = R"""(
class Main {
   function void main() {
      do Output.printInt(1 + (2 * 3));
      return;
   }
}
)""";

const char* LET_STATEMENT_SRC = R"""(
class Main {
    function int f1() {
        var int v1, v2;
        var bool v3;

        let v1 = 1;
        let v2 = 1 + -v2;
        let v3 = true;

        return v1 + Math.pow(v2, 2);
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
