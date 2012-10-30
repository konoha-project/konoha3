//written by Joseph

import("konoha.string");

void test() {
	System.p("こんにちは、世界".charCodeAt(0));
	assert("こんにちは、世界".charCodeAt(0) == 12371);
}

test();

void test1() {
	"こんにちは、世界".charCodeAt(999); // throws Exception
}
