// written by shinpei
//import("konoha.new"); Don't use this
import("dscript.subproc");
import("JavaScript.Array");

void test () {
	Subproc p = new Subproc("echo hoge", false);
	p.enablePipemodeOUT(true);
	p.bg();
	String[] ret = p.communicate("");
	// need enough time
	assert(ret[0] == "hoge\n");
}

test();

