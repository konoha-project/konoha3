//written by takeo onoda

import("Java.String");

void test() {
	assert("foobar".startsWith("foo") == true);
	assert("foobar".startsWith("foobar") == true);
	assert("foobar".startsWith("foobarhoge") == false);
	assert("aあiいuう".startsWith("aあiい") == true);
	assert("aあiいuう".startsWith("aあiいuう") == true);
	assert("aあiいuう".startsWith("aあiいuうe") == false);
	assert("".startsWith("") == true);
}

test();
