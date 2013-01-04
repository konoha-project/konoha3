//Written by Joseph

import("Type.Bytes");
import("posix.socket");

void test() {
	int fd = System.socket(PF_INET, SOCK_STREAM, 0);
	assert(fd > 0);
	assert(System.close(fd) == 0);
}

test();
