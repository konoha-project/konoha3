//Written by Joseph

//import("konoha.new"); Don't use this
import("dscript.subproc");

void test() {
	Subproc s = new Subproc("echo hi", false);
	assert(s.exec("echo \"hi\"") == "\"hi\"\n");
	assert(s.exec("echo hello") == "hello\n");
	s.fg();
	//after execution
	assert(s.exec("ls") == "");
}

test();
