//Written by Joseph
K.import("posix.fd");
K.import("konoha.bytes");
K.import("posix.file");

void test() {
	FILE f = System.fopen("filewrite.txt", "r+");
	if ( System.access("filewrite.txt", F_OK) == -1) {
	assert(f.putC(49) == false);
	assert(f.putC(10) == false);
	}
	f.close();
	f = System.fopen("filewrite.txt", "w+");
	assert(f.putC(49));
	assert(f.putC(10));
	f.close();
	f = System.fopen("filewrite.txt", "r");
	assert(f.putC(49) == false);
	assert(f.putC(10) == false);
	f.close();
	f = System.fopen("filewrite.txt", "w");
	assert(f.putC(49));
	assert(f.putC(10));
	f.close();
}

test();
