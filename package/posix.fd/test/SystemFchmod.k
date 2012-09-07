//Written by Joseph

K.import("posix.fd");
K.import("posix.process");

void test() {
	int fd = System.open("another.txt", O_RDONLY);
	assert(System.fchmod(fd, 0750));
}

test();
