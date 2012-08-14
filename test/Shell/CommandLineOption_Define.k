// written by Takuma Wakamori
// expects:
// $ minikonoha -DIVAL=123 -SVAL="abc"

void test() {
	assert(IVAL == 123);
	assert(SVAL == "abc");
}

test();
