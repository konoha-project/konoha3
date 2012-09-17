//Written by Joseph

import("konoha.int");
import("posix.fd");

void test() {
	int fd = System.open("../", O_RDONLY);
	assert(System.fchdir(fd) == 0);
	fd = System.open("/", O_WRONLY);//fd == -1
	assert(System.fchdir(fd) == -1);
	fd = System.open("../", O_RDWR);//fd == -1
	assert(System.fchdir(fd) == -1);
	fd = System.open("../", O_CREAT|O_EXCL);//fd == -1
	assert(System.fchdir(fd) == -1);
	fd = System.open("../", O_TRUNC);//fd == -1
	assert(System.fchdir(fd) == -1);
	fd = System.open("../", O_APPEND|O_NONBLOCK);
	assert(System.fchdir(fd) == 0);
	fd = System.open("../", O_NOCTTY|O_NDELAY|O_RDONLY);
	assert(System.fchdir(fd) == 0);
	fd = System.open("../", O_NOCTTY|O_WRONLY);
	assert(System.fchdir(fd) == -1);
}

test();
