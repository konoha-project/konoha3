//Written by Joseph

import("posix.path");

void test() {
	int u = System.umask(5);
	assert(System.umask(u) == 5);
}

test();
