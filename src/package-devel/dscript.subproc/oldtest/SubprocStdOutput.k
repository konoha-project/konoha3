// written by shinpei

//import("konoha.new"); Don't use this
import("dscript.subproc");
import("JavaScript.Array");

void test () {
	Subproc p = new Subproc("echo helloworld", false);
	assert(p.bg() == true);
}

test();

