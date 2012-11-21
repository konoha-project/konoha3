// written by masahiro ide

import("konoha.map");
import("konoha.new");
import("konoha.array");

void test() {
	Map[String] m = new Map[String]();
	m.set("abcd", "efgh");
	assert(m.get("abcd") == "efgh");
	m["abcd"] = "0000";
	assert(m.get("abcd") == "0000");
}

test();
