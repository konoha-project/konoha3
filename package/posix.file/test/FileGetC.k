//Written by Joseph
K.import("konoha.bytes");
K.import("posix.file");
K.import("posix.fd");

void test() {
	FILE f = System.fopen("testfile.txt", "r");
	assert(f.getC() == 104);
	assert(f.getC() == 111);
	assert(f.getC() == 103);
	assert(f.getC() == 101);
	f.close();
	f = System.fopen("testfile.txt", "r+");
	assert(f.getC() == 104);
	assert(f.getC() == 111);
	assert(f.getC() == 103);
	assert(f.getC() == 101);
	f.close();
	f = System.fopen("notexist.txt", "w+");
	assert(f.getC() == -1);
	f.close();
	f = System.fopen("notexist.txt", "w");
	assert(f.getC() == -1);
	f.close();
	f = System.fopen("notexist.txt", "a");
	assert(f.getC() == -1);
	f.close();
	System.unlink("notexist.txt");
}

test();
