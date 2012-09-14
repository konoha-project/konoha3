//Written by Joseph
K.import("konoha.bytes");
K.import("posix.socket");

void test() {
	int fd = System.socket(PF_INET, SOCK_DGRAM, 0);
	System.bind(fd, "hoge", 9999, AF_INET);
	System.listen(fd, SOMAXCONN);
	System.shutdown(fd, SHUT_RDWR);
	System.close(fd) == 0;
}
test();
