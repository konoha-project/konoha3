// written by kimio

import("cstyle");

void test() {
	int i;
	while(i < 10) {
		i = i + 1;
	}
	assert(i == 10);
}

test();
