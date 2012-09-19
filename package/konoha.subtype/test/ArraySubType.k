// written by kimio

import("konoha.subtype");

void test() {
	String[] o;
	assert(!(o <: boolean[]));
	assert(!(o <: int[]));
	assert((o <: String[]));
	assert(!(o <: System[]));
	assert(!(o <: Func[]));
}

test();

