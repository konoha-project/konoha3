//written by takeo onoda

import("konoha.string");

void test() {
	assert("fOoBaR123".toLower() == "foobar123");
	assert("aあiいuう".toLower() == "aあiいuう");
}

test();
