//written by takeo onoda

import("konoha.string");

void test() {
	assert("foobar".endsWith("bar") == true);
	assert("foobar".endsWith("foobar") == true);
	assert("foobar".endsWith("0foobar") == false);
	assert("aあiいuう".endsWith("iいuう") == true);
	assert("aあiいuう".endsWith("aあiいuう") == true);
	assert("aあiいuう".endsWith("0aあiいuう") == false);
	assert("".endsWith("") == true);
}

test();
