// Written by Joseph
// Modified by Kimio

import("posix.stat");

void test() {
	FileStatus s = System.lstat("/etc/passwd");
	assert(s.getuid() > 0);
	assert(s.getgid() > 0);
	assert(s.getsize() > 0);
}

test();
