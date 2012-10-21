//Written by Joseph

import("konoha.new");
import("konoha.array");
import("dscript.subproc");

void test() {
	Subproc s = new Subproc(["echo", "hello"], false);
	assert(s.getReturncode() == 0);
}

test();
