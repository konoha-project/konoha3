// written by wakamori

int String.g(int i) {
	return i + 1;
}

void test() {
	assert("abc".g(1) == 2);
}

test();
