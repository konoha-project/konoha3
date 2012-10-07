//Written by Joseph

import("konoha.bytes");
import("posix.socket");

void test() {
	int fd = System.socket(PF_INET, SOCK_STREAM, 0);
	/* bind successes on Linux, fails on Mac OS X */
	System.bind(fd, "hoge", 9999, AF_INET);
	System.close(fd);
}

test();
