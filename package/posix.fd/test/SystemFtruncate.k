//Written by Joseph

K.import("posix.fd");
K.import("posix.process");

void test() {
	if (System.access("ftruncate.txt", F_OK) != 0) {
		p("ftruncate.txt is not exist. touch ftruncate.txt");
		//TODO:Create a file.
		//int fd = System.open("ftruncate.txt", O_WRONLY|O_CREAT|O_TRUNC, 0666);
	} else {
		int fd = System.open("ftruncate.txt", O_WRONLY|O_TRUNC);
		assert(System.ftruncate(fd, 10));
		assert(System.ftruncate(fd, 30));
		fd = System.open("ftruncate.txt", O_RDWR);
		assert(System.ftruncate(fd, 20));
		fd = System.open("ftruncate.txt", O_RDONLY);
		assert(System.ftruncate(fd, 5) == false);
	}
}

test();
