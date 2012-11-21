// Written by Joseph
// Modified by Kimio

import("posix.path");

void test() {
	FileStatus s = System.stat("/etc/passwd");
	assert(s.getdev() > 0);
	assert(s.getino() > 0);
	assert(s.getmode() > 0);
	assert(s.getnlink() > 0);
}

test();
