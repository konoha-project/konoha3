//written by Joseph

import("Java.String");

void test() {
	assert("canal".lastIndexOf("a") == 3);
	assert("canal".lastIndexOf("a", 2) == 1);
	assert("canal".lastIndexOf("a", 0) == -1);
	assert("canal".lastIndexOf("x") == -1);
	assert("canal".lastIndexOf("CANAL") == -1);
	assert("Hello planet earth, you are a great planet.".lastIndexOf("planet") == 36);
	assert("Hello planet earth, you are a great planet.".indexOf("planet") == 6);
	assert("Hello planet earth, you are a great planet.".lastIndexOf("earth") == 13);
	assert("Hello planet earth, you are a great planet.".indexOf("earth") == 13);
}

test();
