// written by kimio kuramitsu

import("Syntax.CStyleFor");

void test() {
	int i = 0;
	int c = 0;
	for(i = 0; i < 10; i = i + 1) {
		if(i % 2 == 0) continue;
		c = c + 1;
	}
	assert(c == 5);
}
test();

