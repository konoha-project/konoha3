// written by kimio

import("dscript.subproc");
import("Type.File");
import("JavaScript.String");
import("JavaScript.Array");

void test() {
	// ls -la | cat -n
	SubProc p = new SubProc("ls");
	p.setArgumentList("-la".split());
	SubProc p2 = new SubProc("cat");
	p2.setArgumentList("-n".split());
	p.setInputStream(stdin);
	p.bg();
	p.pipe(p2, false);
	p2.setOutputStream(stdout);
	p2.setErrorStream(stdout);
	p2.fg();
}

test();


