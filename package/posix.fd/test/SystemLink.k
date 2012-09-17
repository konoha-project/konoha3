//Written by Joseph

import("posix.fd");

void test() {
	assert(System.link("files/newfile.txt", "files/linkafter.txt") == 0);
	assert(System.unlink("files/linkafter.txt") == 0);
	assert(System.link("files/notexist.txt", "files/linkafter.txt") == -1);
	assert(System.unlink("files/notexist.txt") == -1);
}

test();
