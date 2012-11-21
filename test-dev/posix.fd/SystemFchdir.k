//Written by Joseph

import("cstyle"); // konoha.int was obsolete
import("posix.fd");

void test() {
	int fd = System.open("../", O_RDONLY);
	assert(System.fchdir(fd) == true);
	fd = System.open("/", O_WRONLY);//fd == -1
	assert(System.fchdir(fd) == false);
	fd = System.open("../", O_RDWR);//fd == -1
	assert(System.fchdir(fd) == false);
	fd = System.open("../", O_CREAT|O_EXCL);//fd == -1
	assert(System.fchdir(fd) == false);
	fd = System.open("../", O_TRUNC);//fd == -1
	assert(System.fchdir(fd) == false);
	fd = System.open("../", O_APPEND|O_NONBLOCK);
	assert(System.fchdir(fd) == true);
	fd = System.open("../", O_NOCTTY|O_NDELAY|O_RDONLY);
	assert(System.fchdir(fd) == true);
	fd = System.open("../", O_NOCTTY|O_WRONLY);
	assert(System.fchdir(fd) == false);
}

test();
