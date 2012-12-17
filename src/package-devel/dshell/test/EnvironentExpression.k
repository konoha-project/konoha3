// written by chenji

import("dshell");

void test() {
	assert(${SHELL} == System.getenv("SHELL"));
	assert($SHELL == System.getenv("SHELL"));
}

test();
