//Written by Joseph

K.import("posix.fd");
K.import("posix.process");

void test() {
	int fd = System.open("newfile.txt", O_WRONLY);
	assert(System.chown("newfile.txt", -1, -1) == 0);
	assert(System.lchown("newfile.txt", -1, -1) == 0);
	assert(System.fchown(fd, -1, -1) == 0);
}

test();
