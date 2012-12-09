import("konoha.new");
import("konoha.namespace");  // konoha.const is obsolete
import("konoha.bytes");
import("konoha.string");
import("konoha.file");
import("dscript.subproc");
import("jansson");

const BUFSIZ = 8;

void sendSignalEvent() {
	Json sdata = new Json();
	sdata.setString("event", "hoge");
	String req = sdata.dump();
	FILE mailbox = new FILE("/usr/local/minikonoha/dse/mailbox", "a");
	req = req + "\n";
	int len = req.getSize();
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
