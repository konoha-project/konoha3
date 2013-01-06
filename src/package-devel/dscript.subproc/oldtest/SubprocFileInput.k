// written by shinpei
//import("konoha.new"); Don't use this
import("dscript.subproc");
import("JavaScript.Array");

void test () {
	Subproc p = new Subproc("cat", false);
	p.enablePipemodeOUT(true);
	FILE f = System.fopen("files/helloworld.txt", "r");
	p.setFileIN(f);
	p.fg();
	// need enough time
	String[] ret = p.communicate("");
	assert(ret[0] == "hello world!\n");
}

test();

