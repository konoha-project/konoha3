//Written by Joseph

//import("konoha.new"); Don't use this
import("dscript.subproc");

void test() {
	Subproc s = new Subproc("echo hello", false);
	assert(s.getReturncode() == 0);
}

test();
