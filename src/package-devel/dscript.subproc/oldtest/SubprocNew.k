//Written by Joseph

//import("konoha.new"); Don't use this
import("JavaScript.Array");
import("dscript.subproc");

void test() {
	Subproc a = new Subproc("echo \"hello\"", false);
	a.enablePipemodeOUT(true);
	assert(a.bg());
	String[] str1 = a.communicate("");
	assert(str1[0] == "\"hello\"\n");

	Subproc b = new Subproc("echo \"hi\"", true);
	b.enablePipemodeOUT(true);
	assert(b.fg() == 0);
	String[] str2 = b.communicate("");
	assert(str2[0] == "\"hi\"\n");

	Subproc c = new Subproc("", false);
	assert(c.fg() != 0);
}

test();
