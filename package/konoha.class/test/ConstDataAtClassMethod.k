// written by Atsushi Uchida
K.import("konoha.class");
A = 10;
B = "abcd";
class C {
    void f() {
        assert(A == 10);
        assert(B == "abcd");
    }
}

new C().f();
