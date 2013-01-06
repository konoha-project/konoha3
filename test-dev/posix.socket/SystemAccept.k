//Written by Joseph

import("Type.Bytes");
//import("konoha.new"); Don't use this
import("posix.socket");

void test() {
	int fd = System.socket(AF_INET, SOCK_STREAM, 0);
	int ret = System.bind(fd, "127.0.0.1", 10000, AF_INET);
	ret = System.listen(fd, SOMAXCONN);
	assert(ret == 0);
	SockAddr remoteInfo = new SockAddr();
	ret = System.accept(fd, remoteInfo);
	assert(ret != -1);
	System.close(fd);
}

test();
