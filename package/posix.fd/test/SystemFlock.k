//Written by Joseph

import("posix.fd");
import("posix.process");

void test() {
	int fd = System.open("files/newfile.txt", O_RDWR);
	assert(System.flock(fd, LOCK_SH));
	assert(System.flock(fd, LOCK_UN));
	assert(System.flock(fd, LOCK_EX));
	assert(System.flock(fd, LOCK_UN));
//	assert(System.flock(fd, LOCK_EX|LOCK_NB));
}

test();
