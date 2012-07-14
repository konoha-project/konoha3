// written by shinpei
K.import("konoha.assignment");
K.import("konoha.while");
K.import("konoha.string");
K.import("konoha.bytes");
K.import("posix.file");

void f () {
	FILE f = System.fopen("../../../package/posix.file/test/Debug/helloworld.txt", "r");
	int bufsize = 32;
	Bytes buf = Bytes.new(bufsize);
	int ret = bufsize;
	String str = "";
	while(ret == bufsize) {
		buf.setAll(0); // zero clear
		ret = f.read(buf, 0, bufsize);
		if (ret < bufsize) {
			String tmp = buf.asString();
			str = str + tmp.substring(0, ret);
		} else {
			str = str + buf.asString();
		}
	}
	p(str);
}
f();

