// written by kimio kuramitsu

import("Syntax.CStyleFor");

void test() {
	int i = 0;
	for(i = 0; i < 10; i = i + 1) i = i + 1;
	assert(i == 10);
}
test();

