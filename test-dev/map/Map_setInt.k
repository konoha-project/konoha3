// written by masahiro ide

import("Konoha.Map");
import("JavaScript.Array");

void test() {
	Map[int] m = new Map[int]();
	m.set("abcd", 100);
	assert(m.get("abcd") == 100);
	m["abcd"] = 200;
	assert(m.get("abcd") == 200);
}

test();
