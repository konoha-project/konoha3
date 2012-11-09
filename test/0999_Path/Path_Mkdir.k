//Written by Joseph

import("posix.path");

void test() {
	assert(System.mkdir("fuga", 0777));
	assert(System.rmdir("fuga"));
}

test();
