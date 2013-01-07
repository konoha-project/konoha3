//Written by Joseph

//import("konoha.new"); Don't use this
import("dscript.subproc");

void test() {
	Subproc s = new Subproc("echo \"hi\"", false);
	//before execution
	assert(s.isPipemodeERR() == false);
	assert(s.enablePipemodeERR(true));
	assert(s.isPipemodeERR());
	assert(s.enablePipemodeERR(false));
	assert(s.isPipemodeERR() == false);
	s.fg();
	//after execution
	assert(s.enablePipemodeERR(true) == false);
	assert(s.enablePipemodeERR(false) == false);
	assert(s.isPipemodeERR() == false);
}

test();
