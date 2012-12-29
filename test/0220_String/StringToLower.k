//written by takeo onoda

import("JavaScript.String");

void test() {
	assert("fOoBaR123".toLower() == "foobar123");
	assert("aあiいuう".toLower() == "aあiいuう");
}

test();
