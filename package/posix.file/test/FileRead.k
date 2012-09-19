// written by shinpei
import("konoha.assign");
import("konoha.while");
import("konoha.string");
import("konoha.bytes");
import("posix.file");

void f () {
	FILE f = System.fopen("../../../package/posix.file/test/helloworld.txt", "r");
	int bufieldsize = 32;
	Bytes buf = Bytes.new(bufieldsize);
	int ret = bufieldsize;
	String str = "";
	while(ret == bufieldsize) {
		buf.setAll(0); // zero clear
		ret = f.read(buf, 0, bufieldsize);
		if (ret < bufieldsize) {
			String tmp = buf.asString();
			str = str + tmp.substring(0, ret);
		} else {
			str = str + buf.asString();
		}
	}
	System.p(str);
}
f();

