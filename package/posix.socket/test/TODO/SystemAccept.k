//Written by Joseph
K.import("konoha.bytes");
K.import("posix.socket");

void test() {
	int fd = System.socket(PF_INET, SOCK_STREAM, 0);
	SockAddr s = SockAddr.new();
	System.listen(fd, SOMAXCONN);
	assert(System.accept(fd, s) > 0);//TODO System.accept doesn't return if the queue is empty.
	System.close(fd);
}
test();
