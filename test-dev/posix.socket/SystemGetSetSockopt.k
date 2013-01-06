//Written by Joseph

import("Type.Bytes");
import("posix.socket");

void test() {
	int fd = System.socket(PF_INET, SOCK_STREAM, 0);
	assert(System.setsockopt(fd, SO_REUSEADDR, 1) == 0);
	//TODO: System.getsockopt returns non 1 value on MacOSX.
	assert(System.getsockopt(fd, SO_REUSEADDR) > 0);
	System.close(fd);
}

test();
