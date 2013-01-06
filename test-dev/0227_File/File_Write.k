//Written by shinpei
//Modified by Joseph

import("Type.Bytes");
import("Java.String");
import("posix.file");

void test() {
	FILE f = System.fopen("files/writetest.txt", "w");
	String str = "1234567890abcde\n";	//the length of str is 16
	int len = str.getSize();
	assert(f.write(str.asBytes(), 0, len) == 16);
	f.close();
}

test();
