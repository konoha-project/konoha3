//Written by Joseph

import("konoha.bytes");
import("posix.file");

void test() {
	assert(System.mkdir("fuga", 0777) == 0);
	assert(System.rmdir("fuga") == 0);
}

test();
