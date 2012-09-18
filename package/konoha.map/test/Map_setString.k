// written by masahiro ide

K.import("konoha.map");
K.import("konoha.new");
K.import("konoha.array");

void test() {
	Map[String] m = new Map[String]();
	m.set("abcd", "efgh");
	assert(m.get("abcd") == "efgh");
	m["abcd"] = "0000";
	assert(m.get("abcd") == "0000");
}

test();
