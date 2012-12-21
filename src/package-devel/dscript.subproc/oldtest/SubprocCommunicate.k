//Written by Joseph
//Modified by Wakamori

import("konoha.new");
import("cstyle"); // konoha.null is obsolate; use cstyle
import("js4.array");
import("dscript.subproc");

void test() {
	Subproc s = new Subproc("echo hi", false);
	s.enablePipemodeOUT(true);
	s.bg();
	String[] str = s.communicate("");
	assert(str[0] == "hi\n");
	assert(str[1].isNull());

	s = new Subproc("echo hello", false);
	s.enablePipemodeOUT(true);
	s.bg();
	String[] str = s.communicate("");
	assert(str[0] == "hello\n");
	assert(str[1].isNull());
}

test();
