K.import("json");
K.import("konoha");

void test() {
	String sample = "{}"

	JSON parsed = JSON.parse(sample);

	// Set JSON

	// int -> json
	parsed.setInt("int", 10);

	// float -> json
	parsed.setFloat("float", 1.0);

	// String -> json
	parsed.setString("string", "hoge");

	// Boolean -> json
	parsed.setBool("bool", true);

	// JSON -> json
	JSON json = new JSON();
	json.setString("foo", "bar");
	json.setString("hoge", "fuga");
	parsed.set("json", json);

	// JSON[] -> json
	JSON[] ja = new JSON[0];
	json = new JSON();
	json.setString("foo", "bar");
	ja.add(json);
	json = new JSON();
	json.setString("hoge", "fuga");
	ja.add(json);
	parsed.setArray("array", ja);

	// Get JSON
	p(parsed.getInt("int"));
	p(parsed.getFloat("float"));
	p(parsed.getString("string"));
	p(parsed.getBool("bool"));
	p(parsed.get("json"));
	JSON[] a = parsed.getArray("array");
	int i = 0;
	p(a.getSize());
	while (i < a.getSize()) {
		p(a[i].dump());
		i += 1;
	}
}

test();
