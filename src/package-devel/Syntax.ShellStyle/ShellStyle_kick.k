import("Syntax.GlobalVariable");
import("posix.process");
import("dscript.subproc");
import("JavaScript.Array");
import("Type.File");
import("Syntax.StringInterpolation");

int lastExitStatus = 0;

@Public String FILE.readAll() {
	String ret = "";
	String line = this.readLine();
	while(line != null) {
		ret = ret + line + "\n";
		line = this.readLine();
	}
	if(ret == "") {
		return ret;
	}
	return ret.substr(0, ret.getlength() - 1);
}

@Public int getExitStatus() {
	return global.lastExitStatus;
}
