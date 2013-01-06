//Written by shinpei
//Modified by Joseph

//import("konoha.new"); Don't use this
import("Type.Bytes");
import("posix.file");

void test() {
	FILE f = System.fopen("files/testfile.txt", "r");
	//the data of this file is "hoge\n" and the size is 5
	int size = 32;
	Bytes buf = new Bytes(size);
	buf.setAll(0);
	assert((f.read(buf, 0, size)) == 5);
	f.close();
}

test();
