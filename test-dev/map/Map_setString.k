// written by masahiro ide

import("Konoha.Map");
import("JavaScript.Array");

void test() {
	Map[String] m = new Map[String]();
	m.set("abcd", "efgh");
	assert(m.get("abcd") == "efgh");
	m["abcd"] = "0000";
	assert(m.get("abcd") == "0000");
}

test();
