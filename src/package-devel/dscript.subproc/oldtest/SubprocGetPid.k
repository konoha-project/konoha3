//Written by Joseph

//import("konoha.new"); Don't use this
import("dscript.subproc");

void test() {
	Subproc s = new Subproc("echo \"hi\"", false);
	//before execution
	assert(s.getPid() == -1);
	s.fg();
	//after execution
	assert(s.getPid() > 0);

	s = new Subproc("echo \"hello\"", true);
	//before execution
	assert(s.getPid() == -1);
	s.bg();
	//after execution
	assert(s.getPid() > 0);
}

test();
