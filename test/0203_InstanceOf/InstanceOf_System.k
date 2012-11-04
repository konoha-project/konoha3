// written by kimio kuramitsu

import("konoha.object");

void test() {
	System o;
	assert(!(o <: boolean));
	assert(!(o <: int));
	assert(!(o <: String));
	assert((o <: System));
	assert(!(o <: String[]));
	assert(!(o <: Func));
}

test();

