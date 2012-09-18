// written by kimio kuramitsu

K.import("konoha.object");
K.import("konoha.subtype");

void test() {
	System sys;
	Object o = sys;
	assert(!(o <: boolean));
	assert(!(o <: int));
	assert(!(o <: String));
	assert((o <: System));
	assert(!(o <: String[]));
	assert(!(o <: Func));
}

test();

