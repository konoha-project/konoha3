// written by kimio kuramitsu

import("konoha.object");
import("konoha.subtype", "<:");

void test() {
	Object o;
	assert(o <: Object);
	assert(!(o <: int));
	assert(!(o <: String));
	assert(!(o <: String[]));
}

test();

