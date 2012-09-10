K.import("konoha.bytes");
K.import("dscript.subproc");

void f () {
	Subproc s = Subproc.new("", false);
	s.enableShellmode(false);
	String str = s.exec("echo $HOME");
	p(str);
}

f();

