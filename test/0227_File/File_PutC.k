//Written by Joseph

import("posix.fd");
import("konoha.bytes");
import("konoha.file");

void test() {
	String file = FILE.scriptPath("files/opentest.txt");
	FILE f = new FILE(file, "r+");
	f.putc(49);
	f.putc(10);
	f.close();
	FILE f = new FILE(file, "w+");
	f.putc(49);
	f.putc(10);
	f.close();
	FILE f = new FILE(file, "w");
	f.putc(49);
	f.putc(10);
	f.close();
	//FILE f = new FILE(file, "r");
	//f.putc(49);
	//f.putc(10);
	//f.close();
}

test();
