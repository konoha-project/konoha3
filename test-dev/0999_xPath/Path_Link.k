//Written by Joseph

import("posix.path");

void test() {
	assert(System.link("files/newfile.txt", "files/linkafter.txt"));
	assert(System.unlink("files/linkafter.txt"));
	assert(!System.link("files/notexist.txt", "files/linkafter.txt"));
	assert(!System.unlink("files/notexist.txt"));
}

test();
