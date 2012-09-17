//Written by Joseph

import("posix.fd");
import("konoha.new");

void test() {
	Stat s = System.lstat("files/newfile.txt");
	assert(s.getst_uid() > 0);
	assert(s.getst_gid() > 0);
	assert(s.getst_size() > 0);
//	Stat v = System.lstat("files/notexist.txt");//SEGV
//	assert(v.getst_dev() == -1);
}

test();
