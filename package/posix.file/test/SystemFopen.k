//Written by Joseph
K.import("konoha");
K.import("konoha.bytes");
K.import("posix.file");
K.import("posix.fd");

void test() {
	FILE f = System.fopen("testfile.txt", "r");
	assert(f.isNotNull());
	f.close();
	assert(f.isNull());
	f = System.fopen("testfile.txt", "w");
	assert(f.isNotNull());
	f.close();
	assert(f.isNull());
	f = System.fopen("testfile.txt", "a");
	assert(f.isNotNull());
	f.close();
	assert(f.isNull());
	f = System.fopen("notexist.txt", "r");
	assert(f.isNull());
	f.close();
	f = System.fopen("notexist.txt", "w");//Create the file.
	assert(f.isNotNull());
	f.close();
	f = System.fopen("notexist.txt", "a");//Create the file.
	assert(f.isNotNull());
	f.close();
	System.unlink("notexist.txt");
}

test();
