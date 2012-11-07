// written by chenji

import("konoha.file");
import("posix.fd");

void test() {
	FILE f = new FILE("/dev/null", "w");
	assert(System.isatty(f.getfileno()) == false);
}

test();
