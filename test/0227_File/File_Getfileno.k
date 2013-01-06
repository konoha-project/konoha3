// written by chenji

import("Type.File");

void test() {
	assert(stdin.getfileno() != -1);
	assert(stdout.getfileno() != -1);
	assert(stderr.getfileno() != -1);
}

test();
