//Written by Joseph

//import("konoha.new"); Don't use this
import("dscript.subproc");

void test() {
	Subproc s = new Subproc("echo hi", false);
	//before execution
	assert(s.setBufsize(32));
	s.bg();
	//after execution
	assert(s.setBufsize(64) == false);
}

test();
