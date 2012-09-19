import("konoha.class");
import("konoha.while");
import("konoha.int");
import("konoha.float");
import("konoha.string");
import("konoha.array");
import("konoha.assign");

K.import("jansson");

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
		p(keys[i]);
		i += 1;
	}
}

main();
