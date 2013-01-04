//Written by Joseph

import("Type.Bytes");
import("posix.socket");

void test() {
	int fd = System.socket(PF_INET, SOCK_STREAM, 0);
	/* bind successes on Linux, fails on Mac OS X */
	int ret = System.bind(fd, "hoge", 9999, AF_INET);
	assert(ret == 0);
	System.close(fd);
}

test();
