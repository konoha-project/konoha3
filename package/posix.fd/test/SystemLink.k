//Written by Joseph

K.import("posix.fd");

void test() {
	assert(System.link("newfile.txt", "linkafter.txt") == 0);
	assert(System.unlink("linkafter.txt") == 0);
	assert(System.link("notexist.txt", "linkafter.txt") == -1);
	assert(System.unlink("notexist.txt") == -1);
}

test();
