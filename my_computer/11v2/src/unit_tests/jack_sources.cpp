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
   method void draw() {
      return;
   }

   method void run() {
      do draw();
      return;
   }
}
)""";
