//Written by Joseph

K.import("posix.fd");
K.import("posix.process");

void test() {
	int fd = System.open("newfile.txt",O_RDWR);
	Stat s = System.fstat(fd);
	assert(s.getst_blksize() > 0);
	assert(s.getst_blocks() > 0);
	assert(s.getst_atime() > 0);
	assert(s.getst_mtime() > 0);
	assert(s.getst_ctime() > 0);
}

test();
