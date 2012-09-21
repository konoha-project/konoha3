//Written by Joseph

import("posix.fd");

void test() {
	if (System.access("files/symlinked.txt", F_OK) == 0) {
		System.unlink("files/symlinked.txt");
	}
	assert(System.symlink("files/newfile.txt", "files/symlinked.txt") == 0);
	assert(System.symlink("files/newfile.txt", "files/newfile.txt") == -1);
	assert(System.symlink("files/notexist.txt", "files/symlinked.txt") == -1);
}

test();
