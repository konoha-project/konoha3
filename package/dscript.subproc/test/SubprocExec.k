//Written by Joseph

import("konoha.new");
import("dscript.subproc");

void test() {
	Subproc s = new Subproc("", false);
	assert(s.exec("echo \"hi\"") == "\"hi\"\n");
	assert(s.exec("echo hello") == "hello\n");
}

test();
