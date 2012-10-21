// written by kimio kuramitsu

import("konoha.var");
import("konoha.null");
import("konoha.object");
import("konoha.subtype", "<:");
void test() {
	var o = null;
	assert(o <: Object);
}

test();


