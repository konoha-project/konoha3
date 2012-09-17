//Written by Joseph

import("posix.fd");

void test() {
	if (System.rename("files/newfile.txt", "files/renamed.txt") < 0) {
		System.p("newfile.txt is not exist. touch newfile.txt");
	}
	assert(System.rename("files/renamed.txt", "files/newfile.txt") == 0);
}

test();
