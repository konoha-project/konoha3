//Written by Joseph

import("konoha.posix");

void test() {
	assert(System.truncate("files/truncate.txt", 10));
	assert(System.truncate("files/truncate.txt", 30));
	assert(!System.truncate("files/truncate.txt", -5));
	assert(!System.truncate("files/notexist.txt", 5));
	assert(!System.truncate("files/notexist.txt", -5));
}

test();
