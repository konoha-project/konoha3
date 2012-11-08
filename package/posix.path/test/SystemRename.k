//Written by Joseph

import("posix.path");

void test() {
	assert(System.rename("files/testfile.txt", "files/destfile.txt"));
	assert(System.rename("files/destfile.txt", "files/testfile.txt"));
}

test();
