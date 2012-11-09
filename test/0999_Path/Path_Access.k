//Written by Joseph

import("cstyle"); // konoha.int was obsolete
import("posix.path");

void test() {
	assert(System.access("files/newfile.txt", R_OK));
	assert(System.access("files/newfile.txt", R_OK|W_OK));
	assert(!System.access("files/newfile.txt", R_OK|W_OK|X_OK));
	assert(System.access("files/newfile.txt", R_OK|W_OK|F_OK));
	assert(!System.access("files/notexist.txt", F_OK));
}

test();
