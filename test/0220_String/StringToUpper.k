//written by takeo onoda

import("JavaScript.String");

void test() {
	assert("fOoBaR123".toUpper() == "FOOBAR123");
	assert("aあiいuう".toUpper() == "AあIいUう");
}

test();
