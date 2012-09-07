//Written by Joseph

K.import("posix.fd");

void test() {
	assert(System.access("newfile.txt", R_OK) == 0);
	assert(System.access("newfile.txt", R_OK|W_OK) == 0);
	assert(System.access("newfile.txt", R_OK|W_OK|X_OK) == 0);
	assert(System.access("newfile.txt", R_OK|W_OK|X_OK|F_OK) == 0);
	assert(System.access("notexist.txt", F_OK) == -1);
//TODO:When you don't have the permission to access the file, return -1.
}
test();
