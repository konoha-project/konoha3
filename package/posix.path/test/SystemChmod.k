//Written by Joseph

import("posix.path");

void test() {
	assert(System.chmod("files/testfile.txt", 0750));
	assert(System.chmod("files/testfile.txt", 0664));
}

test();
