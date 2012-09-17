//Written by Joseph

import("konoha.bytes");
import("posix.file");

void test() {
	int u = System.umask(5);
	assert(System.umask(u) == 5);
}

test();
