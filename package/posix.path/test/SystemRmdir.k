//Written by Joseph

import("posix.path");

void test() {
	System.mkdir("hoge", 0777);
	assert(System.rmdir("hoge"));
	assert(!System.rmdir("notexist"));
}

test();
