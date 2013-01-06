// written by chenji

import("Type.File");
import("posix.fd");

void test() {
	assert(System.isatty(stdin.getfileno()) == true);
	assert(System.isatty(stdout.getfileno()) == true);
	assert(System.isatty(stderr.getfileno()) == true);
	FILE f = new FILE("/dev/null", "w");
	assert(System.isatty(f.getfileno()) == false);
}

test();
