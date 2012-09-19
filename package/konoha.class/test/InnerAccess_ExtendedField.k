// written by kimio kuramitsu

import("konoha.class");
import("konoha.assign");

class C {
	int x;
	int y;
}

class D extends C {
	int z;
	void f() {
		x = 1;
		y = 2;
		z = 3;
	}
}

void test() {
	D d = new D();
	d.f();
	assert(d.x == 1);
	assert(d.y == 2);
	assert(d.z == 3);
}

test();

