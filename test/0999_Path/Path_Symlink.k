//Written by Joseph

import("posix.path");

void test() {
	assert(System.symlink("files/newfile.txt", "files/symlinked.txt"));
	assert(!System.symlink("files/newfile.txt", "files/newfile.txt"));
	assert(!System.symlink("files/notexist.txt", "files/symlinked.txt"));
}

test();
