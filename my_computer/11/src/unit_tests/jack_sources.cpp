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

const char* SINGLE_RETURN_SRC = R"""(
class Main {
    function int f1() {
        return 1;
    }
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

const char* CLASS_METHOD_CALL_SRC = R"""(
class Main {
    method int mul(int a) {
        return 0;
    }

    method int f1() {
        return mul(1, 2);
    }
}
)""";
#if 0
class Main {
    field int v1, v2;

    method int mul(int n) {
        return v1 * n;
    }

    method int f1(int a1) {
        let v1 = a1;

        return mul(2);
    }
}
#endif
