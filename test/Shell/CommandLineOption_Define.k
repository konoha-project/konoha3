// written by Takuma Wakamori
// expects:
// $ minikonoha -DIVAL=123 -DSVAL=abc

void test() {
	assert(IVAL == 123);
	assert(SVAL == "abc");
}

test();
