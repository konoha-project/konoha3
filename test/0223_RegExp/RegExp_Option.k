import("JavaScript.Regexp");

void test() {
	assert("hello".search(/[ABCDE]/i) == 1);
	assert("hello".search(/[ABCDE]/) == -1);
}

test();
