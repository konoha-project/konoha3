// written by kimio kuramitsu

import("cstyle");

void test() {
	int i = 0;
	do {i = i + 1; } while(i < 10); 
	assert(i == 10);
}
test();

