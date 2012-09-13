//Written by Joseph
K.import("konoha.bytes");
K.import("posix.file");

void test() {
	assert(System.mkdir("fuga", 0777) == 0);
	assert(System.rmdir("fuga") == 0);
}

test();
