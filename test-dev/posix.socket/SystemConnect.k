//Written by Joseph
/*
   This script should be executed after running SystemAccept.k
   */
import("Type.Bytes");
import("posix.socket");

void test() {
	int fd = System.socket(AF_INET, SOCK_STREAM, 0);
	int ret = System.bind(fd, "", 9999, AF_INET);
	assert(ret == 0);
	ret = System.connect(fd, "127.0.0.1", 10000, AF_INET);
	assert(ret == 0);
	System.close(fd);
}

test();
