//Written by Joseph

//import("konoha.new"); Don't use this
import("dscript.subproc");

void test() {
	Subproc s = new Subproc("echo \"hi\"", false);
	//before execution
	assert(s.isStandardIN() == false);
	assert(s.enableStandardIN(true));
	assert(s.isStandardIN());
	assert(s.enableStandardIN(false));
	assert(s.isStandardIN() == false);
	s.fg();
	//after execution
	assert(s.enableStandardIN(true) == false);
	assert(s.enableStandardIN(false) == false);
	assert(s.isStandardIN() == false);
}

test();
