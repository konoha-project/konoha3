//Written by Joseph

K.import("posix.process");

void test() {
	assert(System.chroot("/home") == 0);//TODO:Need the root permission to change root.
}

test();
