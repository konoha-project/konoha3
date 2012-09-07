//Written by Joseph

K.import("posix.fd");
K.import("posix.process");

void test() {
	int fd = System.open("newfile.txt", O_RDWR);
	assert(System.lseek(fd, 10, SEEK_SET) == 10);
	assert(System.lseek(fd, 500, SEEK_END) == 521);
	assert(System.lseek(fd, -1, SEEK_SET) == -1);
	assert(System.lseek(fd, -100, SEEK_CUR) > 0);
}

test();
