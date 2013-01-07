//Written by Joseph

//import("konoha.new"); Don't use this
import("dscript.subproc");

void test() {
	Subproc s = new Subproc("echo \"hi\"", false);
	//before execution
	assert(s.isStandardOUT() == false);
	assert(s.enableStandardOUT(true));
	assert(s.isStandardOUT());
	assert(s.enableStandardOUT(false));
	assert(s.isStandardOUT() == false);
	s.fg();
	//after execution
	assert(s.enableStandardOUT(true) == false);
	assert(s.enableStandardOUT(false) == false);
	assert(s.isStandardOUT() == false);
}

test();
