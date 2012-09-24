//Written by Joseph

import("konoha.new");
import("dscript.subproc");

void test() {
	Subproc s = new Subproc("", true);
	assert(s.isPipemodeOUT() == false);

	s = new Subproc("", false);
	assert(s.isPipemodeOUT() == false);

	s = new Subproc("ls", true);
	assert(s.isPipemodeOUT() == false);

	s = new Subproc("ls", false);
	assert(s.isPipemodeOUT() == false);
}

test();
