// written by shinpei
//import("konoha.new"); Don't use this
import("dscript.subproc");

void test () {
	Subproc p = new Subproc("echo hoge", false);
	FILE f = System.fopen("files/output.txt", "w");
	p.setFileOUT(f);
	assert(p.fg() == 0);
	// need enough time
	f.close();
}
test();

