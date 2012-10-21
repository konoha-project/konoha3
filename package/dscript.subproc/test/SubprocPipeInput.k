// written by shinpei

import("konoha.new");
import("konoha.array");
import("dscript.subproc");

// echo 'hello world' | cat
void test() {
	Subproc p = new Subproc(["cat"], false);
	p.enablePipemodeOUT(true);
	p.enablePipemodeIN(true);
	p.bg();
	// need enough time
	String[] ret = p.communicate("hello world!\n");
	assert(ret[0] == "hello world!\n");
}

test();

