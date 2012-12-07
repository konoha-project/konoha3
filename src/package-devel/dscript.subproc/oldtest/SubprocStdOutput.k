// written by shinpei

import ("konoha.new");
import( "dscript.subproc");
import ("konoha.array");

void test () {
	Subproc p = new Subproc("echo helloworld", false);
	assert(p.bg() == true);
}

test();

