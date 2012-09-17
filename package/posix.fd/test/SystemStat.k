//Written by Joseph

import("posix.fd");
import("konoha");
import("konoha.new");

void test() {
	Stat s = System.stat("files/newfile.txt");
	assert(s.getst_dev() > 0);
	assert(s.getst_ino() > 0);
	assert(s.getst_mode() > 0);
	assert(s.getst_nlink() > 0);
//	Stat v = System.stat("notexist.txt");//SEGV
//	assert(v.getst_dev() == -1);
}

test();
