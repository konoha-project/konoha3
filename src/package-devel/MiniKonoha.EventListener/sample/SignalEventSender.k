//import("konoha.new"); Don't use this
import("MiniKonoha.NameSpace");  // konoha.const is obsolete
import("Type.Bytes");
import("Java.String");
import("Type.File");
import("dscript.subproc");
import("Type.Json");

const BUFSIZ = 8;

void sendSignalEvent() {
	Json sdata = new Json();
	sdata.setString("event", "hoge");
	String req = sdata.toString();
	FILE mailbox = new FILE("/usr/local/minikonoha/dse/mailbox", "a");
	req = req + "\n";
	int len = req.length();
	mailbox.write(req.toBytes());
	mailbox.close();
	FILE pid = new FILE("/usr/local/minikonoha/dse/pid", "r");
	Bytes buf = new Bytes(BUFSIZ);
	buf.setAll(0);
	pid.read(buf);
	pid.close();
	Subproc s = new Subproc("kill -USR1 " + new String(buf), false);
	s.fg();
}

void test() {
	sendSignalEvent();
}

test();
