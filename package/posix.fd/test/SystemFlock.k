//Written by Joseph

K.import("posix.fd");
K.import("posix.process");

void test() {
	int fd = System.open("newfile.txt", O_RDWR);
	assert(System.flock(fd, LOCK_SH));
	assert(System.flock(fd, LOCK_UN));
	assert(System.flock(fd, LOCK_EX));
	assert(System.flock(fd, LOCK_EX));
}

test();
