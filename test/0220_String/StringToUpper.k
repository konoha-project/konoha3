//written by takeo onoda

import("konoha.string");

void test() {
	assert("fOoBaR123".toUpper() == "FOOBAR123");
	assert("aあiいuう".toUpper() == "AあIいUう");
}

test();
