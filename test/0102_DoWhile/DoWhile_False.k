// written by kimio kuramitsu

import("Syntax.CStyleWhile");

void test() {
	int i = 0;
	do {
		i = i + 1;
	} while(false);
	assert(i == 1);
}
test();

