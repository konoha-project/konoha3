// written by shinpei

//import("konoha.new"); Don't use this
import("dscript.subproc");
import("JavaScript.Array");

void test () {
	Subproc p = new Subproc("echo $SHELL", false);
	p.enablePipemodeOUT(true);
	p.fg();
	String[] ret = p.communicate("");
	assert(ret[0] == "$SHELL\n");
}

test();

