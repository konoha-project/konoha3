// written by kimio kuramitsu

import("konoha.untyped");
import("cstyle"); // konoha.null is obsolate; use cstyle
import("konoha.object");
import("konoha.subtype", "<:");

void test() {
	o = null;
	assert(o <: Object);
}

test();


