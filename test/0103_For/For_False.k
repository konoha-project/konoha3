// written by kimio kuramitsu

import("cstyle");

void test() {
	int i = 0;
	for(i = 0; false; i = i + 1) {
	}
	//System.p(i);
	assert(i == 0);
}
test();

