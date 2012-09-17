//Written by Joseph

import("posix.fd");

void test() {
	int fd = System.open("files/ftruncate.txt", O_WRONLY);
	assert(System.ftruncate(fd, 10));
	assert(System.ftruncate(fd, 30));
	fd = System.open("files/ftruncate.txt", O_RDWR);
	assert(System.ftruncate(fd, 20));
	fd = System.open("files/ftruncate.txt", O_RDONLY);
	assert(System.ftruncate(fd, 5) == false);
}

test();
