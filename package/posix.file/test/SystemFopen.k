//Written by Joseph

import("konoha");
import("konoha.bytes");
import("posix.file");
import("posix.fd");

void test() {
	FILE f = System.fopen("files/opentest.txt", "r");
	assert(f.isNotNull());
	f.close();
	f = System.fopen("files/opentest.txt", "w");
	assert(f.isNotNull());
	f.close();
	f = System.fopen("files/opentest.txt", "a");
	assert(f.isNotNull());
	f.close();
	f = System.fopen("files/notexist.txt", "r");//fail to open
	f.close();
	f = System.fopen("files/notexist.txt", "w");//Create the file.
	assert(f.isNotNull());
	f.close();
	f = System.fopen("files/notexist.txt", "a");//Create the file.
	assert(f.isNotNull());
	f.close();
	System.unlink("files/notexist.txt");
}

test();
