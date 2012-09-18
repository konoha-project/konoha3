K.import("json");
K.import("konoha");

void test() {
	String sample = "{\"int\": 10, \"float\": 1.0, \"string\": \"hoge\", \"bool\": true, \"array\": [{\"foo\": \"bar\"}, {\"hoge\": \"huga\"}], \"json\": {\"foo\": \"bar\", \"hoge\": \"huga\"}}"

	JSON parsed = JSON.parse(sample);
	p(parsed.getInt("int"));
	p(parsed.getFloat("float"));
	p(parsed.getString("string"));
	p(parsed.getBool("bool"));
	JSON[] a = parsed.getArray("array");
	int i = 0;
	while (i < a.getSize()) {
		p(a[i]);
		i += 1;
	}
	System.p(parsed.get("json"));
}

test();
