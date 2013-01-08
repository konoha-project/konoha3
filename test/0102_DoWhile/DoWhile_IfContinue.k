// written by kimio kuramitsu

import("Syntax.CStyleWhile");

void test() {
	int i = 0;
	int c = 0;
	do {
		i = i + 1;
		if(i % 2 == 0) continue;
		c = c + 1;
	} while(i < 10);
	// System.p("c="+c+",i="+i);
	assert(c == 5);
}
test();

