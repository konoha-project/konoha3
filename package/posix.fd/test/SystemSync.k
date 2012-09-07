//Written by Joseph

K.import("posix.fd");
K.import("posix.process");

void test() {
	int fd = System.open("newfile.txt",O_RDONLY);
	assert(System.sync(fd));
	assert(System.fsync(fd) == 0);
}

test();
