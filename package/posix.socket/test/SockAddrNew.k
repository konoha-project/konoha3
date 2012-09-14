//Written by Joseph
K.import("konoha.bytes");
K.import("posix.socket");

void test() {
	SockAddr s = new SockAddr();
	assert(s.isNotNull());
	assert(SockAddr.new().isNotNull());
}
test();
