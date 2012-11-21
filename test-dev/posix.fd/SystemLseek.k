//Written by Joseph

import("posix.fd");
import("posix.process");

void test() {
	int fd = System.open("files/newfile.txt", O_RDWR);
	assert(System.lseek(fd, 10, SEEK_SET) == 10);
	assert(System.lseek(fd, 500, SEEK_END) == 527);
	assert(System.lseek(fd, -1, SEEK_SET) == -1);
	assert(System.lseek(fd, -100, SEEK_CUR) > 0);
}

test();
