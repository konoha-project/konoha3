// written by shinpei
//import("konoha.new"); Don't use this
import("dscript.subproc");
import("JavaScript.Array");

// echo 'hello world' | cat
void test () {
	Subproc p = new Subproc("cat", false);
	p.enablePipemodeOUT(true);
	p.enablePipemodeIN(true);
	p.bg();
	// need enough time
	String[] ret = p.communicate("hello world!\n");
	assert(ret[0] == "hello world!\n");
}

test();

