const char* JACK_SEVEN_SRC = R"""(
class Main {
   function void main() {
      do Output.printInt(1 + (2 * 3));
      return;
   }
}
)""";

const char* CLASSVAR_SRC = R"""(
class testjack {
    static ClassName inst1;
    field int var1;
    field boolean var2;
}
)""";

const char* CLASSANDSUBVARS_SRC = R"""(
class JackTest {
    static ClassName inst1;
    field int var1;
    field boolean var2;

    method void sub1(int a1, int a2) {
        var int v1;
        var boolean v2, v3;

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
