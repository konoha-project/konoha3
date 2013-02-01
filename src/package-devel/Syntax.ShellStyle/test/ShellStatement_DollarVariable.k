// written by chenji

import("Syntax.ShellStyle");

void test() {
	int i = 123;
	String s = "abc";
	assert($(echo ${i}) == "123\n");
	assert($(echo ${s}) == "abc\n");
}

test();
