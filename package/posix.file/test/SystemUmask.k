//Written by Joseph
K.import("konoha.bytes");
K.import("posix.file");

void test() {
	int u = System.umask(5);
	assert(System.umask(u) == 5);
}

test();
