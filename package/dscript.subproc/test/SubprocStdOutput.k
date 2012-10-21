// written by shinpei

import("konoha.new");
import("konoha.array");
import("dscript.subproc");

void test () {
	Subproc p = new Subproc(["echo", "helloworld"], false);
	assert(p.bg() == true);
}

test();

