//Written by Joseph

import("Type.File");
import("Type.Bytes");

void test() {
	FILE f = System.fopen("files/opentest.txt", "r+");
	if( System.access("files/opentest.txt", F_OK) == -1) {
	assert(f.putC(49) == false);
	assert(f.putC(10) == false);
	}
	f.close();
	f = System.fopen("files/opentest.txt", "w+");
	assert(f.putC(49));
	assert(f.putC(10));
	f.close();
	f = System.fopen("files/opentest.txt", "r");
	assert(f.putC(49) == false);
	assert(f.putC(10) == false);
	f.close();
	f = System.fopen("files/opentest.txt", "w");
	assert(f.putC(49));
	assert(f.putC(10));
	f.close();
}

test();
