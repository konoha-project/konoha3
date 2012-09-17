//Written by Joseph

import("posix.process");

void test() {
	assert(System.fork() >= 0);//child process get 0 as return, parent process get child pid as return.
}

test();
