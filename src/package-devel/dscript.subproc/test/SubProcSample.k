// written by kimio

import("dscript.subproc");
import("Type.File");

void test() {
	SubProc p = new SubProc("ls");
	p.setInputStream(stdin);
	p.setOutputStream(stdout);
	p.setErrorStream(stdout);
	p.fg();
}

test();


