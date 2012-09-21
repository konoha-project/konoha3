//Written by Joseph

import("konoha.int");
import("posix.process");

void test() {
	assert(System.chdir("../") == 0);
	assert(System.chdir("/") == 0);
	assert(System.chdir("../notexist") == -1);
	assert(System.chdir("~/") == -1);
}

test();
