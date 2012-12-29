//written by takeo onoda

import("JavaScript.String");

void test() {
	assert("hoge".get(0) == "h");
	assert("hoge".get(3) == "e");
	//assert("hoge".get(4));
	assert("aあiいuう".get(0) == "a");
	assert("aあiいuう".get(5) == "う");
	//assert("aあiいuう".get(6));
	//assert("".get(1));
}

test();
