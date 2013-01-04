//Written by Joseph

//import("konoha.new"); Don't use this
import("dscript.subproc");

void test() {
	Subproc s = new Subproc("echo hi", false);
	assert(s.bg());
	assert(s.getPid() > 0);
	assert(s.bg() == false);

	s = new Subproc("echo hello", true);
	assert(s.bg());
	assert(s.getPid() > 0);
	assert(s.bg() == false);

	s = new Subproc("", false);
	assert(s.getPid() == -1);
	assert(s.bg() == false);

	s = new Subproc("", true);
	assert(s.getPid() == -1);
	assert(s.bg() == false);
}

test();
