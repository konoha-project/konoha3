// written by kimio

import("konoha.subtype");

void test() {
	boolean b;
	int n;
	String s;
	Func f;
	assert(!(b <: void));
	assert(!(n <: void));
	assert(!(s <: void));
	assert(!(f <: void));
}

test();

