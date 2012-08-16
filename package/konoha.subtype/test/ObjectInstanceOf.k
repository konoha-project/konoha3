// written by kimio kuramitsu

K.import("konoha.class");

void test() {
	Object o;
	assert(o <: Object);
	assert(!(o <: Int));
	assert(!(o <: String));
	assert(!(o <: String[]));
}

test();

