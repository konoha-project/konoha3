//written by Joseph

import("konoha.string");

void test() {
	assert("こんにちは、世界".charCodeAt(0) == 12371);
	"こんにちは、世界".charCodeAt(999); // throws Exception
}

test();
