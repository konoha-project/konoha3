// written by kimio kuramitsu

import("Syntax.CStyleFor");

void test() {
	int c = 0;
	for(int i = 0; i < 10; i = i + 1) {
		c = c + 1;
	}
	assert(c == 10);
}
test();

