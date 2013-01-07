//written by takeo onoda

import("JavaScript.String");

void test() {
	assert("hoge".getlength() == 4);
	assert("aあiいuう".getlength() == 6);
	assert("".getlength() == 0);
}

test();
