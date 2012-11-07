// written by chenji

import("konoha.file");

void test() {
	FILE f = new FILE("/dev/null", "w");
	assert(f.isatty() == false);
}

test();
