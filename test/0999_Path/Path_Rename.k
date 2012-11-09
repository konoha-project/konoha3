//Written by Joseph

import("posix.path");

void test() {
	assert(System.rename("files/renamed.txt", "files/newfile.txt"));
}

test();
