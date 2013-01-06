// written by kimio kuramitsu

import("Syntax.CStyleWhile");

void test() {
	int i = 0;
	int c = 0;
	while(i < 10) {
		int j = 0;
		while(j < 10) {
			j = j + 1;
			c = c + 1;
		}
		i = i + 1;
	}
	assert(c == 100);
}
test();

