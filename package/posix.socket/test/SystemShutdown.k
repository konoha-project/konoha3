//Written by Joseph
K.import("konoha.bytes");
K.import("posix.socket");

void test() {
	int fd = System.socket(PF_INET, SOCK_STREAM, 0);
	System.listen(fd, SOMAXCONN);
	assert(System.shutdown(fd, SHUT_RDWR) == 0);
	System.close(fd) == 0;
}
test();
