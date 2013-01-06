//Written by Joseph

//import("konoha.new"); Don't use this
import("dscript.subproc");

void test() {
	Subproc s = new Subproc("echo \"hi\"", false);
	//before execution
	assert(s.isStandardERR() == false);
	assert(s.enableStandardERR(true));
	assert(s.isStandardERR());
	assert(s.enableStandardERR(false));
	assert(s.isStandardERR() == false);
	s.fg();
	//after execution
	assert(s.enableStandardERR(true) == false);
	assert(s.enableStandardERR(false) == false);
	assert(s.isStandardERR() == false);
}

test();
