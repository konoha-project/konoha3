// written by kimio

boolean isTrue() {
	return true;
}

boolean isFalse() {
	return false;
}

void test() {
	boolean btrue = true;
	boolean bfalse = false;
	int a = 1;
	assert(true != false)
	assert(false != true);
	assert(btrue != false);
	assert(bfalse != true);
	assert(a < 2 != false);
	assert(a > 2 != true);
	assert(isTrue() != false);
	assert(isFalse() != true);
}

test()

