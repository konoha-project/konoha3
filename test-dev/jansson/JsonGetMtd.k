import("konoha.new");
import("cstyle"); // konoha.int was obsolete
import("konoha.float");
import("java2.string");
import("konoha.array");
// //import("konoha.assign"); Don't use any more
import("cstyle", "while");

import("jansson");

void main() {
	String sample = "{\"int\": 10, \"float\": 1.0, \"string\": \"hoge\", \"bool\": true, \"array\": [{\"foo\": \"bar\"}, {\"hoge\": \"huga\"}], \"json\": {\"foo\": \"bar\", \"hoge\": \"huga\"}}"
	/*
	sample = {
		"int": 10,
		"float": 1.0,
		"string": "hoge",
		"bool": true,
		"array": [
			"foo": "bar"},
			{"hoge": "huga"}
		], 
		"json": {
			"foo": "bar",
			"hoge": "huga"
		}
	}
	*/

	Json parsed = Json.parse(sample);
	System.p(parsed.getInt("int"));
	System.p(parsed.getFloat("float"));
	System.p(parsed.getString("string"));
	System.p(parsed.getBool("bool"));
	Json[] a = parsed.getArray("array");
	int i = 0;
	while (i < a.getSize()) {
		System.p(a[i]);
		i += 1;
	}
	System.p(parsed.getJson("json"));
	String[] keys = parsed.getKeys();
	i = 0;
	while(i < keys.getSize()) {
		System.p(keys[i]);
		i += 1;
	}
}

main();
