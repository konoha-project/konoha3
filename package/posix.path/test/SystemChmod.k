//Written by Joseph

void test() {
	assert(System.chmod("files/testfile.txt", 0750) == 0);
	assert(System.chmod("files/testfile.txt", 0664) == 0);
}

test();
