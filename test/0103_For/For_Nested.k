// written by kimio kuramitsu

import("Syntax.CStyleFor");

void test() {
	int i = 0;
	int c = 0;
	for(i = 0; i < 10; i = i + 1) {
		int j = 0;
		for(j = 0; j < 10; j = j + 1) {
			c = c + 1;
		}
	}
	assert(c == 100);
}
test();

