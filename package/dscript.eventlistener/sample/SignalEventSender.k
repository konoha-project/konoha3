import("konoha.new");
import("konoha.const");
import("konoha.bytes");
import("konoha.string");
import("posix.file");
import("dscript.subproc");
import("jansson");

const BUFSIZ = 8;

void sendHttpEvent() {
	Json sdata = new Json();
	sdata.setString("event", "hoge");
	String req = sdata.dump();
	FILE mailbox = System.fopen("/usr/local/minikonoha/dse/mailbox", "a");
	req = req + "\n";
	int len = req.getSize();
	mailbox.write(req.asBytes(), 0, len);
	mailbox.close();
	FILE pid = System.fopen("/usr/local/minikonoha/dse/pid", "r");
	Bytes buf = new Bytes(BUFSIZ);
	buf.setAll(0);
	pid.read(buf, 0, BUFSIZ);
	pid.close();
	Subproc s = new Subproc("kill -USR1 " + buf.asString(), false);
	s.fg();
}

void test() {
	sendHttpEvent();
}

test();
