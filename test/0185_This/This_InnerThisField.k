// written by kimio kuramitsu

//import("konoha.new"); Don't use this
import("Syntax.JavaStyleClass");

class C {
	int x;
	int y;
	C() {}
	void f() {
		x = 1;
		y = 2;
	}

	void g() {
		int y = 0;
		x = 2;
		this.y = 4;
	}
}

void test() {
	C c = new C();
	c.f();
	assert(c.x == 1);
	assert(c.y == 2);
	c.g();
	assert(c.x == 2);
	assert(c.y == 4);
}

test();

