K.import("konoha");
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

	Json j = new Json();
	Json parsed = j.parse(sample);
	p(parsed.getInt("int"));
	p(parsed.getFloat("float"));
	p(parsed.getString("string"));
	p(parsed.getBool("bool"));
	Json[] a = parsed.getArray("array");
	int i = 0;
	while (i < a.getSize()) {
		p(a[i]);
		i += 1;
	}
	p(parsed.get("json"));
	String[] keys = parsed.getKeys();
	i = 0;
	while(i < keys.getSize()) {
		p(keys[i]);
		i += 1;
	}
}

main();
