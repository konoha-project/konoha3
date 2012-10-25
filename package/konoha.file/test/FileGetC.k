//Written by Joseph

import("konoha.bytes");
import("posix.file");
import("posix.fd");

void test() {
	FILE f = new FILE("files/testfile.txt", "r");
	assert(f.getC() == 104);
	assert(f.getC() == 111);
	assert(f.getC() == 103);
	assert(f.getC() == 101);
	assert(f.getC() == 10);
	f.close();
	f = new FILE("files/testfile.txt", "r+");
	assert(f.getC() == 104);
	assert(f.getC() == 111);
	assert(f.getC() == 103);
	assert(f.getC() == 101);
	assert(f.getC() == 10);
	f.close();
	f = new FILE("files/notexist.txt", "w+");
	assert(f.getC() == -1);
	f.close();
	f = new FILE("files/notexist.txt", "w");
	assert(f.getC() == -1);
	f.close();
	f = new FILE("files/notexist.txt", "a");
	assert(f.getC() == -1);
	f.close();
	System.unlink("files/notexist.txt");
}

test();
