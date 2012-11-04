//Written by Joseph

import("cstyle"); // konoha.int was obsolete
import("posix.fd");

void test() {
	assert(System.open("files/openfile.txt", O_RDONLY) > 0);
	assert(System.open("files/openfile.txt", O_WRONLY) > 0);
	assert(System.open("files/openfile.txt", O_RDWR) > 0);
	assert(System.open("files/openfile.txt", O_CREAT|O_EXCL) == -1);
	assert(System.open("files/openfile.txt", O_CREAT, 0644) > 0);
	assert(System.open("files/openfile.txt", O_CREAT|O_EXCL, 0644) == -1);
	assert(System.open("files/openfile.txt", O_CREAT|O_TRUNC, 0644) > 0);
	assert(System.open("files/openfile.txt", O_TRUNC) > 0);
	assert(System.open("files/openfile.txt", O_APPEND) > 0);
	assert(System.open("files/openfile.txt", O_NONBLOCK|O_RDONLY) > 0);
	assert(System.open("files/openfile.txt", O_NDELAY|O_WRONLY) > 0);
	assert(System.open("files/openfile.txt", O_NOCTTY|O_RDWR) > 0);
}

test();
