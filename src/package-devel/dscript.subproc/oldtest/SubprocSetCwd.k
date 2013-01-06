//Written by Joseph

//import("konoha.new"); Don't use this
import("cstyle"); // konoha.null is obsolate; use cstyle
import("dscript.subproc");

void test() {
	Subproc s = new Subproc("echo hi", false);
	//before execution
	assert(s.setCwd("../"));
	assert(s.exec("echo hello") == "hello\n");
	assert(s.setCwd("./fuga"));
	assert(s.exec("echo fmfm") == "");//exec is not executed
	s.bg();
	//after execution
	assert(s.setCwd("./hoge") == false);
	assert(s.exec("echo hoge") == null);
}

test();
