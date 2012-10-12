// written by shinpei

import ("konoha.new");
import("dscript.subproc");

void test () {
	Subproc p = new Subproc("", false);
	p.enableShellmode(false);
	//	p.enablePipemodeOUT(true);
	String ret = p.exec("echo $SHELL");
	assert(ret == "$SHELL\n");
}

test();
