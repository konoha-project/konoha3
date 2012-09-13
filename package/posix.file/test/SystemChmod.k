//Written by Joseph
K.import("konoha.bytes");
K.import("posix.file");

void test() {
	assert(System.chmod("testfile.txt", 0750) == 0);
	assert(System.chmod("testfile.txt", 0664) == 0);
}

test();
