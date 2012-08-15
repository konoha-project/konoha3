// written by kimio kuramitsu

K.import("konoha.class");
K.import("konoha.assign");

class C {
	int x;
	@Final int succ() {
		return x;
	}
}

class D extends C {
	@Override int succ() {
		return x + 1;
	}
}

void test() {
	C c = new C();
	D d = new D();
	assert(c.succ() == 0);
	assert(d.succ() == 0);
}

test();

