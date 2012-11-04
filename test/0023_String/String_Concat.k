// written by kimio kuramitsu

void test() {
	String a = "abc";
	String b = "ABC";
	String c = a + b;
	assert(c == "abcABC");
	assert(a + b == "abcABC");
	assert(a + "ABC" == "abcABC");
	assert("abc" + b == "abcABC");
	assert("abc" + "ABC" == "abcABC");
}

test();
