// written by Takuma Wakamori
// expects:
// $ konoha -DIVAL=123 -DSVAL=abc

void test() {
	assert(IVAL == 123);
	assert(SVAL == "abc");
}

test();
