//Written by Joseph

import("konoha.bytes");
import("posix.fd");

void test() {
	assert(System.readlink("files/readlink.txt") == "newfile.txt");
	assert(System.readlink("files/newfile.txt") == "");
	assert(System.readlink("files/notexist.txt") == "");
}

test();
