// written by kimio kuramitsu

import("Syntax.CStyleWhile");

void test() {
	int i = 0;
	while(false) {
		i = i + 1;
	}
	assert(i == 0);
}
test();

