//Written by Joseph

import("konoha.int");
import("posix.fd");

void test() {
	assert(System.access("files/newfile.txt", R_OK) == 0);
	assert(System.access("files/newfile.txt", R_OK|W_OK) == 0);
	assert(System.access("files/newfile.txt", R_OK|W_OK|X_OK) == -1);
	assert(System.access("files/newfile.txt", R_OK|W_OK|F_OK) == 0);
	assert(System.access("files/notexist.txt", F_OK) == -1);
}

test();
