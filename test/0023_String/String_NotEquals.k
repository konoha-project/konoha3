//written by kimio 

void test() {
	String a = "ABC";
	String b = "abc";
	String c = "ABC";
	assert(a != b);
	assert(a != "abc");
	assert("abc" != a);
	assert(!(a != c));
	assert(!(a != "ABC"));
	assert(!("ABC" != a));
}

test();
