// written by kimio kuramitsu

K.import("konoha");

class C {
	C self;
}

void test() {
	C c = new C();
	assert(c.self.isNull());
}

test();

