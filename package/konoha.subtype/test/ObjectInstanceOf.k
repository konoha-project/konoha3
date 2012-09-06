// written by kimio kuramitsu

K.import("konoha.class");
K.import("konoha.subtype");

void test() {
	Object o;
	assert(o <: Object);
	assert(!(o <: int));
	assert(!(o <: String));
	assert(!(o <: String[]));
}

test();

