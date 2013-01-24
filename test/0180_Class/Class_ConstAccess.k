// written by Atsushi Uchida

//import("konoha.new"); Don't use this
import("MiniKonoha.NameSpace");  // konoha.const is obsolete
import("Syntax.JavaStyleClass");

const A = 10;
const B = "abcd";

class C {
    C() {}
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
