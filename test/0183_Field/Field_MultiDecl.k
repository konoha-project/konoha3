// written by kimio kuramitsu

//import("konoha.new"); Don't use this
import("Syntax.JavaStyleClass");

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
