// written by ide
// modified by chenji
import("Java.String");
import("Syntax.StringInterpolation");

int add(int n) {
    return n + 1;
}

void interpolation() {
    int apples = 4;
    String s = "I have ${apples+1 + add(apples)} apples";
    assert("I have ${apples} apples" == "I have 4 apples");
    assert(s == "I have 10 apples");
    assert("${1} + ${2} = ${1+2}" == "1 + 2 = 3");
    String x = "aaa";
    assert("${x+x+2}${}" == "aaaaaa2");
}

interpolation();

void literal() {
    assert("abcd efgh".length() == 9);
    assert("${".length() == 2);
}

literal();
