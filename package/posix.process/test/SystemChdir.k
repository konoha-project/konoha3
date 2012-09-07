//Written by Joseph

K.import("posix.process");

void test() {
// System.chdir
	assert(System.chdir("../test") == 0);
	assert(System.chdir("/") == 0);
	assert(System.chdir("../notexist") == -1);
	assert(System.chdir("~/") == -1);

// System.fchdir
	int fd = System.open("../", O_RDONLY);
	assert(System.fchdir(fd) == 0);
	fd = System.open("/", O_WRONLY);//fd == -1
	assert(System.fchdir(fd) == -1);
	fd = System.open("../", O_RDWR);//fd == -1
	assert(System.fchdir(fd) == -1);
	fd = System.open("../", O_CREAT);//fd == -1
	assert(System.fchdir(fd) == -1);
	fd = System.open("../", O_TRUNC);//fd == -1
	assert(System.fchdir(fd) == -1);
	fd = System.open("../", O_EXCL);
	assert(System.fchdir(fd) == 0);
	fd = System.open("../", O_APPEND|O_NONBLOCK);
	assert(System.fchdir(fd) == 0);
	fd = System.open("../", O_NOCTTY|O_NDELAY|O_WRONLY);
	assert(System.fchdir(fd) == 0);
	fd = System.open("../", O_NOCTTY|O_WRONLY);
	assert(System.fchdir(fd) == -1);
}

test();
