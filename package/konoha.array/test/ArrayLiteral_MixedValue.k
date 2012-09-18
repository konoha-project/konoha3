// written by kimio kuramitsu

K.import("konoha.array");
K.import("konoha.object");

void test() {
	Object[] a = [1, "2", true]; // boxing
	assert(a.getSize() == 3);
}

test();

