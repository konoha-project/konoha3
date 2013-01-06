// written by kimio kuramitsu

import("Syntax.CStyleWhile");

void test() {
	int i = 0;
	while(i < 10) i = i + 1;
	assert(i == 10);
}
test();

