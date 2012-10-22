//Written by Joseph

import("konoha.new");
import("konoha.null");
import("konoha.array");
import("dscript.subproc");

void test() {
	Subproc s = new Subproc(["pwd"], false);
	assert(s.setCwd("/"));
	s.enablePipemodeOUT(true);
	s.fg();
	assert(s.communicate()[0] == "/\n");
}

test();
