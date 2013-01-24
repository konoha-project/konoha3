import("Syntax.StringInterpolation");

assert("${1+1}" == "2");

void f00() {
    int x = 10;
    assert("${x}" == "10");
}

f00();

void f01() {
    int x = 10;
    assert("${x+10}" == "20");
}
f01();

void f02() {
    int x = 10;
    assert("x${x+10}" == "x20");
}

f02();

void f03() {
    int x = 10;
    assert("aaaaaaaaaa${x+10}" == "aaaaaaaaaa20");
}

f03();

void f04() {
    int x = 10;
    assert("${x+10}aaaaaaaaaa" == "20aaaaaaaaaa");
}

f04();

void f05() {
    int x = 10;
    assert("aaaaaaaaaa${x+10}bbbbbbbbbb" == "aaaaaaaaaa20bbbbbbbbbb");
}

f05();
