//written by takeo onoda

import("Java.String");

void test() {
	assert("foobar".lastIndexOf("f") == 0);
	assert("foobar".lastIndexOf("o") == 2);
	assert("foobar".lastIndexOf("A") == -1);
	assert("foobarfoobar".lastIndexOf("bar") == 9);
	assert("foobarfoobar".lastIndexOf("barr") == -1);
	assert("aあiいuう".indexOf("a") == 0);
	assert("aあiいuう".indexOf("あ") == 1);
	assert("aあiいuう".indexOf("I") == -1);
	assert("aあiいuう".indexOf("いuう") == 3);
	assert("aあiいuう".indexOf("いuうe") == -1);
}

test();
