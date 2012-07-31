// written by kimio kuramitsu

K.import("konoha.class");
K.import("konoha.assign");

class C {
	int x;
	C(int x1) {
		x = x1;
	}
}

void test() {
	C c = new C(1);
	assert(c.x==1);
}

test();


