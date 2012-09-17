//Written by Joseph

import("konoha.bytes");
import("posix.socket");

void test() {
	int fd = System.socket(PF_INET, SOCK_STREAM, 0);
	assert(System.bind(fd, "hoge", 9999, AF_INET) == 0);
	System.close(fd);
}

test();
