// written by Atsushi Uchida
K.import("konoha.class");
K.import("konoha.const");
const A = 10;
const B = "abcd";
class C {
    void f() {
        assert(A == 10);
        assert(B == "abcd");
    }
}

new C().f();
