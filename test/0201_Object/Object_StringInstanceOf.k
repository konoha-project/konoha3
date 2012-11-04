// written by kimio kuramitsu

import("konoha.object");

void test() {
	Object o = "";
	assert(o <: Object);
	assert(!(o <: int));
	assert(o <: String);
	assert(!(o <: String[]));
}

test();

