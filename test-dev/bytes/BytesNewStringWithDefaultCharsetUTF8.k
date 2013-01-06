/* written by shinpei nakata */

//import("konoha.new"); Don't use this
import("Type.Bytes");
import("cstyle.libc");

void test() {
	FILE f = System.fopen("files/utf8.txt", "r");
	int size = 32;
	Bytes buf = new Bytes(size);
	buf.setAll(0);
	size = f.read(buf, 0, size);
	buf.set(size, 0);
	String newStr = new String(buf, 0, size);
	assert(newStr == "こんにちは世界\n");
}

test();
