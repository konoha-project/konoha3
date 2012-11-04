// written by kimio kuramitsu

void test() {
	String a = "abc";
	int b = 123;
	String c = a + b;
	assert(c == "abc123");
	assert(a + b == "abc123");
	assert(a + 123 == "abc123");
	assert("abc" + 123 == "abc123");
}

test();
