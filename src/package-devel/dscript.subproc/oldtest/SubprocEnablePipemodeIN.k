//Written by Joseph

//import("konoha.new"); Don't use this
import("dscript.subproc");

void test() {
	Subproc s = new Subproc("echo \"hi\"", false);
	//before execution
	assert(s.isPipemodeIN() == false);
	assert(s.enablePipemodeIN(true));
	assert(s.isPipemodeIN());
	assert(s.enablePipemodeIN(false));
	assert(s.isPipemodeIN() == false);
	s.fg();
	//after execution
	assert(s.enablePipemodeIN(true) == false);
	assert(s.enablePipemodeIN(false) == false);
	assert(s.isPipemodeIN() == false);
}

test();
