//written by takeo onoda

import("Java.String");

void test() {
	assert("foobar".indexOf("f") == 0);
	assert("foobar".indexOf("o") == 1);
	assert("foobar".indexOf("A") == -1);
	assert("foobar".indexOf("bar") == 3);
	assert("foobar".indexOf("barr") == -1);
	assert("aあiいuう".indexOf("a") == 0);
	assert("aあiいuう".indexOf("あ") == 1);
	assert("aあiいuう".indexOf("I") == -1);
	assert("aあiいuう".indexOf("いuう") == 3);
	assert("aあiいuう".indexOf("いuうe") == -1);
}

test();
