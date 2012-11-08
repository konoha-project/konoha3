//Written by shinpei
//Modified by Joseph

import("konoha.object");
import("konoha.bytes");
import("konoha.string");
import("konoha.file");

void test() {
	String file = FILE.scriptPath("files/writetest.txt");
	FILE f = new FILE(file, "w");
	String str = "1234567890abcde\n";	//the length of str is 16
	int len = str.getSize();
	assert(f.write((str to Bytes), 0, len) == 16);
	f.close();
}

test();
