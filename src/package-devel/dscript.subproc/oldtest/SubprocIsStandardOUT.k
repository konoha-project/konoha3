//Written by Joseph

//import("konoha.new"); Don't use this
import("dscript.subproc");

void test() {
	Subproc s = new Subproc("", true);
	assert(s.isStandardOUT() == false);

	s = new Subproc("", false);
	assert(s.isStandardOUT() == false);

	s = new Subproc("ls", true);
	assert(s.isStandardOUT() == false);

	s = new Subproc("ls", false);
	assert(s.isStandardOUT() == false);
}

test();
