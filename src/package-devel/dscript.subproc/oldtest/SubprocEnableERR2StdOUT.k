//Written by Joseph
//Modified by Wakamori

//import("konoha.new"); Don't use this
import("JavaScript.Array");
import("dscript.subproc");

void test() {
	Subproc s = new Subproc("echo hi 3>&1 1>&2 2>&3", false);
	s.enableShellmode(true);
	//before execution
	assert(s.isERR2StdOUT() == false);
	assert(s.enableERR2StdOUT(true));
	assert(s.isERR2StdOUT());
	assert(s.enableERR2StdOUT(false));
	assert(s.isERR2StdOUT() == false);
	s.enableERR2StdOUT(true);
	s.enablePipemodeOUT(true);
	s.bg();
	String[] str = s.communicate("");
	assert(str[0] == "hi\n");
	//after execution
	assert(s.enableERR2StdOUT(true) == false);
	assert(s.enableERR2StdOUT(false) == false);
	assert(s.isERR2StdOUT() == true);
}

test();
