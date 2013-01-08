// written by kimio kuramitsu

import("Syntax.CStyleWhile");

void test() {
	int i = 0;
	int c = 0;
	do {
		int j = 0;
		do {
			j = j + 1;
			c = c + 1;
		} while(j < 10);
		i = i + 1;
	} while(i < 10);
	assert(c == 100);
}
test();

