//Written by Joseph

K.import("posix.fd");

void test() {
	if (System.rename("newfile.txt", "renamed.txt") < 0) {
		p("newfile.txt is not exist. touch newfile.txt")
	}
	assert(System.rename("renamed.txt", "newfile.txt") == 0);
}

test();
