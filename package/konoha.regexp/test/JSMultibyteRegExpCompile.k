//written by Joseph

K.import("konoha.regexp");
K.import("konoha.assign");
K.import("konoha.string");

void test() {
	String str = "世界、こんにちは、世界!";
	RegExp patt = /世界/g;
	assert(str.replace(patt, "日本") == "日本、こんにちは、日本!");
	//change and recompile the regular expression "patt"
	patt = /(、)?世界/g;
	assert(str.replace(patt, "日本") == "日本、こんにちは日本!");
}

test();
