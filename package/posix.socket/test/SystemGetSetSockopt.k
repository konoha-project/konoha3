//Written by Joseph
K.import("konoha.bytes");
K.import("posix.socket");

void test() {
	int fd = System.socket(PF_INET, SOCK_STREAM, 0);
	assert(System.setsockopt(fd, SO_REUSEADDR, 1) == 0);
	assert(System.getsockopt(fd, SOL_REUSEADDR) == 0);
	System.close(fd);
}
test();
