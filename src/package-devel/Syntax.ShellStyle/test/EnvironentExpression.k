// written by chenji

import("Syntax.ShellStyle");

void test() {
	assert(${SHELL} == System.getenv("SHELL"));
	assert($SHELL == System.getenv("SHELL"));
}

test();
