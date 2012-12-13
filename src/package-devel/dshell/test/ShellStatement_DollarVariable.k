// written by chenji

import("dshell");

void test1() {
	String s = "abc";
	echo ${s}
}

void test2() {
	int i = 123;
	echo ${i}
}

test1();
test2();
