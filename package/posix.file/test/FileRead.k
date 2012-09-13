//Written by shinpei
//Modified by Joseph
import("konoha.new");
import("konoha.bytes");
import("posix.file");

void test() {
	FILE f = System.fopen("testfile.txt", "r");
	//the data of this file is "1234567890abc\n" and the size is 14
	int size = 32;
	Bytes buf = new Bytes(size);
	buf.setAll(0);
	assert((f.read(buf, 0, size)) == 14);
	f.close();
}

test();
