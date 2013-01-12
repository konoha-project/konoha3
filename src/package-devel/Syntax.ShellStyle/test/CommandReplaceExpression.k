// written by chenji

import("Syntax.ShellStyle");

void test() {
	String s1 = $(echo hi);
	assert(s1 == "hi\n");
	String s2 = $(echo hi\nhello | grep hello);
	assert(s2 == "hello\n");
}

test();
