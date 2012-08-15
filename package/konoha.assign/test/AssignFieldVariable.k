// written by kimio kuramitsu

K.import("konoha.assign");
K.import("konoha.class");

class C {
	int x;
}

void test() {
	C c = new C();
	c.x = 10;
	assert(c.x == 10);
	c.x = 0;
	assert(c.x == 0);
}

test();

