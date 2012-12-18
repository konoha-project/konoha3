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
	assert(true == true)
	assert(false == false);
	assert(btrue == true);
	assert(bfalse == false);
	assert(a < 2 == true);
	assert(a > 2 == false);
	assert(isTrue() == true);
	assert(isFalse() == false);
}

test()

