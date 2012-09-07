//Written by Joseph

K.import("posix.fd");

void test() {
	if (System.access("symlinked.txt", F_OK) == 0) {
		System.unlink("symlinked.txt");
	}
	assert(System.symlink("newfile.txt", "symlinked.txt") == 0);
	assert(System.symlink("newfile.txt", "newfile.txt") == -1);
	assert(System.symlink("notexist.txt", "symlinked.txt") == -1);
}

test();
