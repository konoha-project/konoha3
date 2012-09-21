//Written by Joseph

import("konoha.bytes");
import("posix.fd");
import("posix.file");

void test() {
	System.mkdir("hoge", 0777);
	assert(System.rmdir("hoge") == 0);
	assert(System.rmdir("notexist") == -1);
}

test();
