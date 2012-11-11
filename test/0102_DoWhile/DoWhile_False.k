// written by kimio kuramitsu

import("cstyle");

void test() {
	int i = 0;
	do {
		i = i + 1;
	}
	while(false);
	assert(i == 1);
}
test();

