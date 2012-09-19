import("konoha.bytes");
import("dscript.subproc");

void f () {
	Subproc s = Subproc.new("", false);
	s.enableShellmode(false);
	String str = s.exec("echo $HOME");
	System.p(str);
}

f();

