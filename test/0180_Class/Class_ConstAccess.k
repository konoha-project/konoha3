// written by Atsushi Uchida

import("konoha.new");
import("konoha.ns");  // konoha.const is obsolete
import("konoha.class");

const A = 10;
const B = "abcd";

class C {
    void f() {
        assert(A == 10);
        assert(B == "abcd");
    }
}

void test() {
	C c = new C();
	c.f();
}

test();
