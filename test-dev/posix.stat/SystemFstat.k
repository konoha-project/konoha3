// Written by Joseph
// Modified by Kimio

import("posix.stat");

void test() {
	FileStatus s = System.fstat(0);
	assert(s.getblksize() > 0);
	assert(s.getblocks() > 0);
	assert(s.getatime() > 0);
	assert(s.getmtime() > 0);
	assert(s.getctime() > 0);
}

test();
