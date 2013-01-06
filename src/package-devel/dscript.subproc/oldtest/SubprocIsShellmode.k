//Written by Joseph

//import("konoha.new"); Don't use this
import("dscript.subproc");

void test() {
	Subproc a = new Subproc("", false);
	assert(a.isShellmode() == false);

	a = new Subproc("", true);
	assert(a.isShellmode() == false);

	a = new Subproc("ls", true);
	assert(a.isShellmode() == false);

	a = new Subproc("ls", false);
	assert(a.isShellmode() == false);
}

test();
