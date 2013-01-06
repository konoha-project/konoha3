//Written by Joseph
//Modified by Wakamori

//import("konoha.new"); Don't use this
import("dscript.subproc");

void test() {
	Subproc s = new Subproc("", false);
	//before execution
	assert(s.isShellmode() == false);
	assert(s.enableShellmode(true));
	assert(s.isShellmode());
	assert(s.enableShellmode(false));
	assert(s.isShellmode() == false);
	assert(s.exec("echo $HOME") == "$HOME\n");
	//after execution
	assert(s.enableShellmode(true) == false);
	assert(s.enableShellmode(false) == false);
	assert(s.isShellmode() == false);
}

test();
