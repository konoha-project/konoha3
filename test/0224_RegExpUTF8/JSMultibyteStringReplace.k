//written by Joseph

import("JavaScript.Regexp");

void test() {
	RegExp patt = /世界/gi;
	assert("せかい、こんにちは、世界".replace(patt, "日本") == "せかい、こんにちは、日本");

	assert("せかい、こんにちは、世界".replace(/世界/gi, "日本") == "せかい、こんにちは、日本");
	assert("せかい、こんにちは、世界".replace(/世界/g, "日本") == "せかい、こんにちは、日本");
	assert("せかい、こんにちは、世界".replace(/世界/i, "日本") == "せかい、こんにちは、日本");
	assert("Hello world! こんにちは 世界!".replace(/(\w+)\s(\w+)/, "$2, $1") == "world, Hello! こんにちは 世界!");
}

test();
