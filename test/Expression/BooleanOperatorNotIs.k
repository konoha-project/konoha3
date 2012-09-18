/* written by kimio kuramitsu */

// this is a bug found in method testing

boolean System.isFalse() {
	return false;
}

void test() {
	assert(!System.isFalse());
}

test();

