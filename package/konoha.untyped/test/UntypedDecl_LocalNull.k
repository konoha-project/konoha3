// written by kimio kuramitsu

import("konoha.untyped");
import("konoha.null");
import("konoha.object");
import("konoha.subtype", "<:");

void test() {
	o = null;
	assert(o <: Object);
}

test();


