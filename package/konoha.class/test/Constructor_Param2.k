// written by kimio kuramitsu

K.import("konoha.class");
K.import("konoha.assign");

class C {
	int x;
	int y;
	C(int x1, int y2) {
		x = x1;
		y = y2;
	}
}

void test() {
	C c = new C(1,2);
	assert(c.x==1);
	assert(c.y==2);
}

test();


