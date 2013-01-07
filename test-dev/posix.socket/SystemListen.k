//Written by Joseph

import("Type.Bytes");
import("posix.socket");

void test() {
	int fd = System.socket(PF_INET, SOCK_STREAM, 0);
	assert(System.listen(fd, SOMAXCONN) == 0);
	System.close(fd);
}

test();
