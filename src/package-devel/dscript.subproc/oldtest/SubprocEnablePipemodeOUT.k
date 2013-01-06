//Written by Joseph

//import("konoha.new"); Don't use this
import("dscript.subproc");

void test() {
	Subproc s = new Subproc("echo \"hi\"", false);
	//before execution
	assert(s.isPipemodeOUT() == false);
	assert(s.enablePipemodeOUT(true));
	assert(s.isPipemodeOUT());
	assert(s.enablePipemodeOUT(false));
	assert(s.isPipemodeOUT() == false);
	s.fg();
	//after execution
	assert(s.enablePipemodeOUT(true) == false);
	assert(s.enablePipemodeOUT(false) == false);
	assert(s.isPipemodeOUT() == false);
}

test();
