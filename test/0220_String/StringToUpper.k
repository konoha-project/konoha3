//written by takeo onoda

import("Java.String");

void test() {
	assert("fOoBaR123".toUpperCase() == "FOOBAR123");
	assert("aあiいuう".toUpperCase() == "AあIいUう");
}

test();
