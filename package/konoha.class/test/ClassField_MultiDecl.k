// written by kimio kuramitsu

K.import("konoha.class");

class A {
	int x, y;
	A () {
	}
}


void f() {
    A a = new A();
    assert(a.x == 0);
    assert(a.y == 0);
}

f();
