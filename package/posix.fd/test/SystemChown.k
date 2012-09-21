//Written by Joseph

import("posix.fd");
import("posix.process");

void test() {
	int fd = System.open("files/newfile.txt", O_WRONLY);
	assert(System.chown("files/newfile.txt", -1, -1) == 0);
	assert(System.lchown("files/newfile.txt", -1, -1) == 0);
	assert(System.fchown(fd, -1, -1) == 0);
}

test();
