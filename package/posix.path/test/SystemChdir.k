//Written by Joseph

import("posix.path");

void test() {
	assert(System.chdir("../"));
	assert(System.chdir("/"));
	assert(!System.chdir("../notexist"));
	assert(!System.chdir("~/"));
}

test();
