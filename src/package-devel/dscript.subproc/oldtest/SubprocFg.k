//Written by Joseph

//import("konoha.new"); Don't use this
import("dscript.subproc");

void test() {
	Subproc s = new Subproc("echo \"hi\"", false);
	assert(s.fg() == 0);
	assert(s.getPid() > 0);
	assert(s.fg() == -400);

	s = new Subproc("echo \"hello\"", true);
	assert(s.fg() == 0);
	assert(s.getPid() > 0);
	assert(s.fg() == -400);

	s = new Subproc("", false);
	assert(s.getPid() == -1);
	assert(s.fg() == -400);

	s = new Subproc("", true);
	assert(s.getPid() == -1);
	assert(s.fg() == -400);
}

test();
