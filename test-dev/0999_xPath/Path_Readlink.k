//Written by Joseph

import("posix.path");

void test() {
	assert(System.readlink("files/readlink.txt") == "newfile.txt");
	assert(System.readlink("files/newfile.txt") == "");
	assert(System.readlink("files/notexist.txt") == "");
}

test();
