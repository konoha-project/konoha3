// written by kimio kuramitsu

import("cstyle");

void test() {
	int i = 0;
	while(false) {
		i = i + 1;
	}
	assert(i == 0);
}
test();

