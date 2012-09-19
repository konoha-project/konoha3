// written by kimio kuramitsu

import("konoha.object");
import("konoha.subtype");

void test() {
	Object o = true;
	assert((o <: boolean));
	assert(!(o <: int));
	assert(!(o <: String));
	assert(!(o <: System));
	assert(!(o <: String[]));
	assert(!(o <: Func));
}

test();

