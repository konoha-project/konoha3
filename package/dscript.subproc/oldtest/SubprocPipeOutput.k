// written by shinpei
import ("konoha.new");
import( "dscript.subproc");
import ("konoha.array");

void test () {
	Subproc p = new Subproc("echo hoge", false);
	p.enablePipemodeOUT(true);
	p.bg();
	String[] ret = p.communicate("");
	// need enough time
	assert(ret[0] == "hoge\n");
}

test();

