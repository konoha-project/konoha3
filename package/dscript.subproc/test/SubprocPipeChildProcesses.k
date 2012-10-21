// written by chenji

import("konoha.array");
import("konoha.new");
import("konoha.file");
import("dscript.subproc");

void test() {
	Subproc s1 = new Subproc("echo hoge\nfuga\nfoo");
	s1.enablePipemodeOUT(true);
	s1.fg();
	Subproc s2 = new Subproc("grep h");
	s2.enablePipemodeOUT(true);
	s2.setFileIN(s1.getFileOUT());
	s2.fg();
	assert(s2.communicate()[0] == "hoge\n");
}

test();
