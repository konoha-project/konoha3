// written by shinpei

//import("konoha.new"); Don't use this
import("dscript.subproc");

void test () {
	Subproc p = new Subproc("", false);
	p.enableShellmode(true);
	String ret = p.exec("echo $SHELL");
	assert(ret == "/bin/bash\n");
}

test();
