//Written by Joseph

import("posix.fd");
import("posix.process");

void test() {
	int fd = System.open("files/newfile.txt", O_RDONLY);
	assert(System.sync(fd));
	assert(System.fsync(fd) == true);
}

test();
